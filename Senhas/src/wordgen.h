#ifndef _OPRP_WORDGEN_
#define _OPRP_WORDGEN_

#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a > b) ? a : b)
#define BASE64_CRYPT                                                           \
  "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"

typedef unsigned long long ull;

static int maxSize = 64;

class Senha {
private:
  char senha[32];
  int vetor[8];

  void avancarN(int n) {
    int overflow, pos = 0;
    vetor[pos] += n;
    while ((overflow = (vetor[pos] / maxSize)) > 0 && pos < 8) {
      vetor[pos] %= maxSize;
      vetor[++pos] += overflow;
    }
  }

public:
  Senha(int comeco) {
    vetor[0] = -1;
    vetor[1] = -1;
    vetor[2] = -1;
    vetor[3] = -1;
    vetor[4] = -1;
    vetor[5] = -1;
    vetor[6] = -1;
    vetor[7] = -1;
    avancarN(comeco);
  }

  Senha() {
    vetor[0] = -1;
    vetor[1] = -1;
    vetor[2] = -1;
    vetor[3] = -1;
    vetor[4] = -1;
    vetor[5] = -1;
    vetor[6] = -1;
    vetor[7] = -1;
  }

  char *getSenha() {
    senha[0] = BASE64_CRYPT[vetor[0]];
    senha[1] = BASE64_CRYPT[vetor[1]];
    senha[2] = BASE64_CRYPT[vetor[2]];
    senha[3] = BASE64_CRYPT[vetor[3]];
    senha[4] = BASE64_CRYPT[vetor[4]];
    senha[5] = BASE64_CRYPT[vetor[5]];
    senha[6] = BASE64_CRYPT[vetor[6]];
    senha[7] = BASE64_CRYPT[vetor[7]];
    return senha;
  }

  void prox() { avancarN(1); }

  void prox(int n) { avancarN(n); }
};

#endif