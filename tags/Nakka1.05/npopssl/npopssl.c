/*
 * npopssl
 *
 * npopssl.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#include <wincrypt.h>

#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/pkcs12.h>

/* Define */
#define MAX_DEPTH			9

/* Global Variables */
static HMODULE crypt32_lib;
static FARPROC _CertOpenSystemStore;
static FARPROC _CertFindCertificateInStore;
static FARPROC _CertCloseStore;

int idx;

typedef struct _SSL_INFO {
	SSL_CTX *ctx;
	SSL *ssl;
	int depth;
} SSL_INFO;

/* Local Function Prototypes */

/*
 * DllMain - メイン
 */
int WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, PVOID pvReserved)
{
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		// SSLの初期化
		SSL_library_init();
		SSL_load_error_strings();
		idx = SSL_get_ex_new_index(0, "App Data Index", NULL, NULL, NULL);
		RAND_poll();
		while (RAND_status() == 0) {
			unsigned short rand_ret = rand() % 65536;
			RAND_seed(&rand_ret, sizeof(rand_ret));
		}

		if ((crypt32_lib = LoadLibrary(TEXT("crypt32.dll"))) != NULL) {
#ifdef _WIN32_WCE
			_CertOpenSystemStore = GetProcAddress(crypt32_lib, TEXT("CertOpenSystemStoreW"));
			_CertFindCertificateInStore = GetProcAddress(crypt32_lib, TEXT("CertFindCertificateInStore"));
			_CertCloseStore = GetProcAddress(crypt32_lib, TEXT("CertCloseStore"));
#else
#ifdef UNICODE
			_CertOpenSystemStore = GetProcAddress(crypt32_lib, "CertOpenSystemStoreW");
#else
			_CertOpenSystemStore = GetProcAddress(crypt32_lib, "CertOpenSystemStoreA");
#endif
			_CertFindCertificateInStore = GetProcAddress(crypt32_lib, "CertFindCertificateInStore");
			_CertCloseStore = GetProcAddress(crypt32_lib, "CertCloseStore");
#endif
		}
		break;

	case DLL_PROCESS_DETACH:
		ERR_free_strings();

		if (crypt32_lib != NULL) {
			FreeLibrary(crypt32_lib);
			crypt32_lib = NULL;
		}
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}

/*
 * verify_callback - verify callback
 */
static int verify_callback(int ok, X509_STORE_CTX *ctx)
{
	SSL_INFO *si;
    SSL *ssl;
    int depth;

	depth = X509_STORE_CTX_get_error_depth(ctx);
	ssl = X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
	if (ssl == NULL) {
		return 0;
	}
    si = SSL_get_ex_data(ssl, idx);
	if (si == NULL) {
		return 0;
	}
 	if (depth > si->depth) {
		ok = 0;
		X509_STORE_CTX_set_error(ctx, X509_V_ERR_CERT_CHAIN_TOO_LONG);
	}
	return ok;
}

/*
 * password_callback - password callback
 */
static int password_callback(char *buf, int size, int rwflag, void *userdata)
{
	if (userdata == NULL) {
		return 0;
	}
	strcpy(buf, userdata);
	return strlen(buf);
}

/*
 * set_client_cert - クライアント証明書と秘密鍵の設定
 */
static int set_client_cert(SSL_CTX *ctx, char *ca_file, char *key_file, char *pass)
{

	if (ca_file != NULL && strlen(ca_file) > 4 &&
		_stricmp(ca_file + (strlen(ca_file) - 4), ".p12") == 0) {
		// PKCS#12
		FILE *fp;
		PKCS12 *p12;
		EVP_PKEY *key = NULL;
		X509 *cert = NULL;

		PKCS12_PBE_add();

		// ファイルの読み込み
		fp = fopen(ca_file, "r");
		if (!fp) {
			return 0;
		}
		// 変換
		p12 = d2i_PKCS12_fp(fp, NULL);
		if (p12 == NULL) {
			fclose(fp);
			return 0;
		}
		fclose(fp);

		// 解析
		if (!PKCS12_parse(p12, pass, &key, &cert, NULL)) {
			return 0;
		}
		if (cert != NULL) {
			if (SSL_CTX_use_certificate(ctx, cert) == 0) {
				X509_free(cert);
				return 0;
			}
			X509_free(cert);
		}
		if (key != NULL) {
			if (SSL_CTX_use_PrivateKey(ctx, key) == 0) {
				EVP_PKEY_free(key);
				return 0;
			}
			EVP_PKEY_free(key);
		}
		PKCS12_free(p12);
	} else {
		// PEM
		if (ca_file != NULL && *ca_file != '\0') {
			if (SSL_CTX_use_certificate_file(ctx, ca_file, X509_FILETYPE_PEM) == 0) {
				return 0;
			}
		}
		if (key_file != NULL && *key_file != '\0') {
			if (SSL_CTX_use_PrivateKey_file(ctx, key_file, X509_FILETYPE_PEM) == 0) {
				return 0;
			}
			if (SSL_CTX_check_private_key(ctx) == 0) {
				return 0;
			}
		}
	}
	return 1;
}

/*
 * load_system_verify - システムに登録されているのCA証明書を取得
 */
static int load_system_verify(SSL_CTX *ssl_ctx)
{
    HCERTSTORE cs;
	X509_STORE *root_certs;
    X509 *cert;
    const CERT_CONTEXT *cert_context = NULL;

	if (_CertOpenSystemStore == NULL ||
		_CertFindCertificateInStore == NULL ||
		_CertCloseStore == NULL) {
		return 0;
	}

	root_certs = X509_STORE_new();
	// 証明書ストアのハンドル取得
	cs = (HCERTSTORE)_CertOpenSystemStore(0, TEXT("ROOT"));

	// 証明書の検索
	while (cert_context = (CERT_CONTEXT *)_CertFindCertificateInStore(cs,
		X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0, CERT_FIND_ANY, NULL, cert_context)) {
		// 変換
		cert = d2i_X509(NULL, (unsigned char **)&cert_context->pbCertEncoded, cert_context->cbCertEncoded);
		if (cert == NULL) {
			return 0;
		}
		X509_STORE_add_cert(root_certs, cert);
		X509_free(cert);
	}
	_CertCloseStore(cs, 0);

	// CTXに登録
	SSL_CTX_set_cert_store(ssl_ctx, root_certs);
	return 1;
}

/*
 * ssl_init - SSL初期化
 */
__declspec(dllexport) long CALLBACK ssl_init(
	const int soc,		// ソケット
	const int ssl_type,	// 0-SSLv23, 1-TLSv1, 2-SSLv3, 3-SSLv2
	int verify,			// 1-VERIFY_PEER, 0-VERIFY_NONE
	int depth,			// 証明書チェーンの長さ
	char *ca_file,		// CA証明書ファイル
	char *ca_path,		// CA証明書パス
	char *cert_file,	// クライアント証明書
	char *key_file,		// 秘密鍵
	char *pass,			// パスワード
	char *err_str)		// (OUT)エラー
{
	SSL_INFO *si;
	SSL_METHOD *sslm;
	long verify_result;
	int ret, e;

	if (ssl_type == -1) {
		return 0;
	}
	si = LocalAlloc(LPTR, sizeof(SSL_INFO));
	if (si == NULL) {
		return 0;
	}
	si->depth = (depth < 0 || depth > MAX_DEPTH) ? MAX_DEPTH : depth;
	// SSL
	switch (ssl_type) {
	case 0:
	default:
		sslm = SSLv23_client_method();
		break;
	case 1:
		sslm = TLSv1_client_method();
		break;
	case 2:
		sslm = SSLv3_client_method();
		break;
	case 3:
		sslm = SSLv2_client_method();
		break;
	}
	si->ctx = SSL_CTX_new(sslm);
	if (si->ctx == NULL){
		ERR_error_string(ERR_get_error(), err_str);
		LocalFree(si);
		return 0;
	}
	SSL_CTX_set_options(si->ctx, SSL_OP_ALL);
	SSL_CTX_set_mode(si->ctx, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
	SSL_CTX_set_default_passwd_cb(si->ctx, password_callback);
	SSL_CTX_set_default_passwd_cb_userdata(si->ctx, pass);
	SSL_CTX_set_verify(si->ctx, (verify != 0) ? SSL_VERIFY_PEER : SSL_VERIFY_NONE, verify_callback);
    SSL_CTX_set_verify_depth(si->ctx, si->depth + 1);

	// クライアント証明書の設定
	if (set_client_cert(si->ctx, cert_file, key_file, pass) == 0) {
		ERR_error_string(ERR_get_error(), err_str);
		SSL_CTX_free(si->ctx);
		LocalFree(si);
		return 0;
	}

	// CA証明書の設定
	if ((ca_file  == NULL && ca_path == NULL) ||
		SSL_CTX_load_verify_locations(si->ctx, ca_file, ca_path) == 0) {
		// システムのCA証明書を設定
		if (load_system_verify(si->ctx) == 0) {
			if (SSL_CTX_set_default_verify_paths(si->ctx) == 0) {
				ERR_error_string(ERR_get_error(), err_str);
				SSL_CTX_free(si->ctx);
				LocalFree(si);
				return 0;
			}
		}
	}

	si->ssl = SSL_new(si->ctx);
	if (si->ssl == NULL){
		ERR_error_string(ERR_get_error(), err_str);
		SSL_CTX_free(si->ctx);
		LocalFree(si);
		return 0;
	}
	SSL_set_ex_data(si->ssl, idx, si);
	if (SSL_set_fd(si->ssl, soc) == 0){
		ERR_error_string(ERR_get_error(), err_str);
		SSL_free(si->ssl);
		SSL_CTX_free(si->ctx);
		LocalFree(si);
		return 0;
	}
	// SSL開始
	while (1) {
		ret = SSL_connect(si->ssl);
		e = SSL_get_error(si->ssl, ret);
		if (e == SSL_ERROR_WANT_CONNECT ||
			e == SSL_ERROR_WANT_READ ||
			e == SSL_ERROR_WANT_WRITE) {
			continue;
		}
		break;
	}
	// 検証結果の取得
	verify_result = SSL_get_verify_result(si->ssl);
	if (ret == -1){
		if (verify_result == X509_V_OK) {
			ret = 0;
			ERR_error_string(ERR_get_error(), err_str);
		} else {
			sprintf(err_str, "%s (%d)", X509_verify_cert_error_string(verify_result), verify_result);
		}
		SSL_free(si->ssl);
		SSL_CTX_free(si->ctx);
		LocalFree(si);
		return ret;
	}
	sprintf(err_str, "%s (%d)", X509_verify_cert_error_string(verify_result), verify_result);
	return (long)si;
}

/*
 * ssl_send - 送信
 */
__declspec(dllexport) int CALLBACK ssl_send(SSL_INFO *si, char *buf, const int len)
{
	if (si == NULL) {
		return -1;
	}
	return SSL_write(si->ssl, buf, len);
}

/*
 * ssl_recv - 受信
 */
__declspec(dllexport) int CALLBACK ssl_recv(SSL_INFO *si, char *buf, const int size)
{
	if (si == NULL) {
		return -1;
	}
	return SSL_read(si->ssl, buf, size);
}

/*
 * ssl_close - 切断
 */
__declspec(dllexport) int CALLBACK ssl_close(SSL_INFO *si)
{
	if (si == NULL) {
		return -1;
	}
	if (si->ssl != NULL) {
		SSL_shutdown(si->ssl);
	}
	return 0;
}

/*
 * ssl_free - 解放
 */
__declspec(dllexport) int CALLBACK ssl_free(SSL_INFO *si)
{
	if (si == NULL) {
		return -1;
	}
	if (si->ssl != NULL) {
		SSL_free(si->ssl);
		si->ssl = NULL;
	}
	if (si->ctx != NULL) {
		SSL_CTX_free(si->ctx);
		si->ctx = NULL;
	}
	LocalFree(si);
	return 0;
}
/* End of source */
