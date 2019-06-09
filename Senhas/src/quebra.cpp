#include "utils.h"
#include "wordgen.h"
#include <algorithm>
#include <crypt.h>
#include <iostream>
#include <map>
#include <math.h>
#include <mpi.h>
#include <omp.h>
#include <set>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

void mpi_sync(int mpi_rank, int mpi_size, MPI_Comm *comm, int num_cifras,
              std::set<int> *falta, int *stop) {
  // Seção de sincronização de progresso
  int *my_list = new int[num_cifras];
  int *other_list = new int[num_cifras];
  std::set<int> my_set;

  fprintf(stderr, "P%d iniciando sincronia MPI\n", mpi_rank);

  while ((*falta).size() > 0 && !stop) {
    // Preparar lista de cifras restantes
    memset(my_list, -1, sizeof(int) * num_cifras);
    memset(other_list, -1, sizeof(int) * num_cifras);
    int cc = 0;
    my_set = std::set<int>((*falta).begin(), (*falta).end());
    for (auto e : my_set) {
      my_list[cc++] = e;
    }

    std::vector<int> new_list;
    sleep_for(4000);
    for (int i = 0; i < mpi_size; i++) {
      if (i == mpi_rank) {
        // Enviar minha lista
        fprintf(stderr, "Processo %d enviando lista\n", mpi_rank);
        MPI_Bcast(my_list, num_cifras, MPI_INT, mpi_rank, *comm);
      } else {
        // Receber lista de alguém
        fprintf(stderr, "Processo %d recebendo lista do processo %d\n",
                mpi_rank, i);
        MPI_Bcast(other_list, num_cifras, MPI_INT, i, *comm);
        std::set<int> other_set;
        for (int j = 0; j < num_cifras; j++) {
          if (other_list[j] > -1)
            other_set.insert(other_list[j]);
        }
        set_intersection(my_set.begin(), my_set.end(), other_set.begin(),
                         other_set.end(), std::back_inserter(new_list));
        my_set = std::set<int>(new_list.begin(), new_list.end());
      }
    }
    printf("[%d] new set: ", mpi_rank);
    for (auto &e : my_set) {
      printf("%d ", e);
    }
    printf("\n");
    (*falta) = std::set<int>(my_set.begin(), my_set.end());
  }
}

int main(int argc, char *argv[]) {
  int thread_level;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &thread_level);

  int mpi_rank = 0;
  int mpi_size = 1;
  MPI_Comm comm = MPI_COMM_WORLD;
  MPI_Comm_rank(comm, &mpi_rank);
  MPI_Comm_size(comm, &mpi_size);

  // Obter comprimento máximo
  int comprimento = 0;
  // int num_threads = 4;
  unsigned long long maximo = 64L;
  if (argc == 2) {
    sscanf(argv[1], "%d", &comprimento);
    // sscanf(argv[2], "%d", &num_threads);
    // printf("p%d argv[1] = %s\n", mpi_rank, argv[1]);
    comprimento = MIN(8, comprimento);
    for (int i = 1; i < comprimento; i++) {
      maximo++;
      maximo *= (unsigned long long)maxSize;
    }
    // printf("p%d max = %llu\n", mpi_rank, maximo);
  } else {
    fprintf(stderr, "Falta argumento: %s <comprimento_maximo> <num_threads>\n",
            argv[0]);
    fprintf(stderr, "Uso: Informe pela entrada padrão o número de cifras, "
                    "número de threads  e em "
                    "seguida digite\n");
    fprintf(stderr, "     as cifras uma por linha.\n");
    exit(1);
  }

  // Ler senhas e sincronizar com outros processos MPI
  int num_cifras = 0;
  std::set<int> falta;
  char **cifras;
  char *cbloco;
  if (mpi_rank == 0) {
    // ROOT
    std::cin >> num_cifras;
    MPI_Bcast(&num_cifras, 1, MPI_INT, 0, comm);
    getchar();
    cifras = new char *[num_cifras];
    cbloco = new char[num_cifras * 32];
    std::string cifra;
    for (int i = 0; i < num_cifras; i++) {
      getline(std::cin, cifra);
      cifras[i] = &cbloco[i * 32];
      strncpy(cifras[i], cifra.data(), 16);
      falta.insert(i);
    }
    MPI_Bcast(cifras[0], num_cifras * 32, MPI_CHAR, 0, comm);
  } else {
    // Not root
    MPI_Bcast(&num_cifras, 1, MPI_INT, 0, comm);
    cifras = new char *[num_cifras];
    cbloco = new char[num_cifras * 32];
    for (int i = 0; i < num_cifras; i++) {
      cifras[i] = &cbloco[i * 32];
      falta.insert(i);
    }
    MPI_Bcast(cifras[0], num_cifras * 32, MPI_CHAR, 0, comm);
  }

  // Iniciar thread de sincronização entre MPI workers
  int stop = 0;
  std::thread sync_thread(mpi_sync, mpi_rank, mpi_size, &comm, num_cifras,
                          &falta, &stop);

  // Usar todos threads disponíveis
  int num_threads = omp_get_max_threads();
  // num_threads = 1;
  omp_set_num_threads(num_threads);
  fprintf(stderr, "p%d Usando %d threads\n", mpi_rank, num_threads);

  unsigned long long i = 0L, counter = 0;
#pragma omp parallel reduction(+ : counter)
  {
    crypt_data myData;
    char *result = myData.crypt_3_buf;
    int thread_rank = omp_get_thread_num();
    int inicio = (mpi_rank * omp_get_num_threads()) + omp_get_thread_num();
    int passo = mpi_size * omp_get_num_threads();
    fprintf(stderr, "p%d t%d inicia em %d (passo %d), existem %d threads\n",
            mpi_rank, thread_rank, inicio, passo, omp_get_num_threads());
    Senha senha(inicio);
    unsigned long long thread_i;
    std::set<int> thread_falta(falta.begin(), falta.end());
    for (thread_i = inicio; thread_i < maximo; thread_i += passo) {
      if ((counter % 10000) == 0) {
#pragma omp barrier
#pragma omp critical(falta_global)
        { thread_falta = std::set<int>(falta.begin(), falta.end()); }

        if ((int)thread_falta.size() == 0) {
          break;
        }
      }
      for (auto &e : thread_falta) {
        // printf("p%d t%d %s == %s?\n", mpi_rank, thread_rank, cifras[e],
        //        senha.getSenha());
        result = crypt_r(senha.getSenha(), cifras[e], &myData);
        int ok = strncmp(result, cifras[e], 14) == 0;

        if (ok) {
          printf("p%*d, t%*d @ %2.f%%: %s = %s\n", (int)ceil(log10(mpi_size)),
                 mpi_rank, (int)ceil(log10(passo)), thread_rank,
                 (thread_i / (double)maximo) * 100, cifras[e],
                 senha.getSenha());
          fflush(stdout);

#pragma omp critical(falta_global)
          {
            if (falta.count(e) > 0) {
              falta.erase(e);
              thread_falta = falta;
            }
          }
        }
      }

      if (((thread_i + 1) % 100000) == 0) {
        fprintf(stderr, "Realizado %2.f%% ou %llu de %llu\n",
                (thread_i / (double)maximo) * 100, thread_i + 1, maximo);
      }

      senha.prox(passo);
      counter++;
    }
  }
  stop = true;
  sync_thread.join();
  fprintf(stderr, "[%d] terminou em %llu iterações!!!!\n", mpi_rank, counter);

  MPI_Finalize();

  return 0;
}
