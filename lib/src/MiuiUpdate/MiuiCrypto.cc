#include "MiuiKeys.h"
#include <MiuiUpdate/MiuiUpdate.h>

#include <openssl/aes.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

std::vector<std::uint8_t> MiuiUpdater::base64Decrypt(const std::string &text) {
  BIO *bmem = BIO_new_mem_buf(text.data(), static_cast<int>(text.size()));
  BIO *b64 = BIO_new(BIO_f_base64());

  if (!bmem || !b64)
    throw std::runtime_error("base64Decrypt: failed to create BIO");

  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
  BIO_push(b64, bmem);

  std::vector<std::uint8_t> buffer(text.size());
  int totalLen = 0;

  while (true) {
    int len = BIO_read(b64, buffer.data() + totalLen,
                       static_cast<int>(buffer.size()) - totalLen);
    if (len <= 0)
      break;
    totalLen += len;
  }

  BIO_free_all(b64);
  buffer.resize(totalLen);
  return buffer;
}

std::string MiuiUpdater::base64Encrypt(std::vector<std::uint8_t> &data) {
  BIO *b64 = BIO_new(BIO_f_base64());
  BIO *bmem = BIO_new(BIO_s_mem());

  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
  BIO_push(b64, bmem);

  BIO_write(b64, data.data(), static_cast<int>(data.size()));
  BIO_flush(b64);

  BUF_MEM *bptr = nullptr;
  BIO_get_mem_ptr(bmem, &bptr);

  std::string result(bptr->data, bptr->length);
  BIO_free_all(b64);

  return result;
}

std::vector<std::uint8_t>
MiuiUpdater::aes128cbcEncrypt(const std::string &data) {
  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  if (!ctx)
    throw std::runtime_error("aes128cbcEncrypt: failed to create context");

  if (EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, MIUI_CRYPTO_KEY,
                         MIUI_CRYPTO_IV) != 1) {
    EVP_CIPHER_CTX_free(ctx);
    throw std::runtime_error("aes128cbcEncrypt: initialization error");
  }

  std::vector<std::uint8_t> ciphertext(data.size() + AES_BLOCK_SIZE);
  int outLen1 = 0, outLen2 = 0;

  if (EVP_EncryptUpdate(ctx, ciphertext.data(), &outLen1,
                        reinterpret_cast<const unsigned char *>(data.data()),
                        static_cast<int>(data.size())) != 1) {
    EVP_CIPHER_CTX_free(ctx);
    throw std::runtime_error("aes128cbcEncrypt: encryption error (Update)");
  }

  if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + outLen1, &outLen2) != 1) {
    EVP_CIPHER_CTX_free(ctx);
    throw std::runtime_error("aes128cbcEncrypt: encryption error (Final)");
  }

  EVP_CIPHER_CTX_free(ctx);
  ciphertext.resize(outLen1 + outLen2);
  return ciphertext;
}

std::string MiuiUpdater::aes128cbcDecrypt(std::vector<uint8_t> &data) {
  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  if (!ctx)
    throw std::runtime_error("aes128cbcDecrypt: failed to create context");

  if (EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, MIUI_CRYPTO_KEY,
                         MIUI_CRYPTO_IV) != 1) {
    EVP_CIPHER_CTX_free(ctx);
    throw std::runtime_error("aes128cbcDecrypt: initialization error");
  }

  std::vector<unsigned char> plaintext(data.size() + AES_BLOCK_SIZE);
  int outLen1 = 0, outLen2 = 0;

  if (EVP_DecryptUpdate(ctx, plaintext.data(), &outLen1, data.data(),
                        static_cast<int>(data.size())) != 1) {
    EVP_CIPHER_CTX_free(ctx);
    throw std::runtime_error("aes128cbcDecrypt: decryption error (Update)");
  }

  if (EVP_DecryptFinal_ex(ctx, plaintext.data() + outLen1, &outLen2) != 1) {
    EVP_CIPHER_CTX_free(ctx);
    throw std::runtime_error(
        "aes128cbcDecrypt: decryption error (Final) — invalid key/IV?");
  }

  EVP_CIPHER_CTX_free(ctx);
  return std::string(plaintext.begin(), plaintext.begin() + outLen1 + outLen2);
}
