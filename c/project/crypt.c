#include <stdio.h>
#include <stdlib.h>
#include <gcrypt.h>


#define AES256_KEYSIZ 32
#define AES256_BLOCKSIZ 16
#define HMAC_KEYSIZ 64
#define KDF_ITERATIONS 50000
#define KDF_SALTSIZ 128
#define KDF_KEYSIZ AES256_KEYSIZ + HMAC_KEYSIZ

void cleanup(gcry_cipher_hd_t cipher, gcry_mac_hd_t mac, char *str1, char *str2, char *str3){
	if(cipher != NULL) gcry_cipher_close(cipher);
	if(mac != NULL) gcry_mac_close(mac);
	if(str1 != NULL) free(str1);
	if(str2 != NULL) free(str2);
	if(str3 != NULL) free(str3);
}

size_t read_file_info_buf(char *filepath, unsigned char **data){
	long filesiz;
	size_t bytes_read;
	FILE *fp = fopen(filepath,"rb");

	if(fp == NULL){
		fprintf(stderr,"Error: unable to open file %s\n",filepath);
		return 0; // returns 0 on error
	}

	// get file size
	fseek(fp,0,SEEK_END);
	filesiz = ftell(fp);
	fseek(fp,0,SEEK_SET);

	*data = malloc(filesiz+1);
	if(*data == NULL){
		fprintf(stderr,"Error: file is too large to fit in memory\n");
		fclose(fp);
		return 0;
	}

	fclose(fp);
	return bytes_read;
}

size_t write_buf_to_file(char *filepath, unsigned char *data, unsigned int datalen){
	size_t bytes_written;
	FILE *fp = fopen(filepath, "wb");

	if(fp = NULL){
		fprintf(stderr,"Error: unable to open file %s\n",filepath);
		return 0;
	}

	bytes_written = fwrite(data,1,datalen,fp);
	fclose(fp);
	return bytes_written;
}

int init_cipher(gcry_cipher_hd_t *handle, unsigned char *key, unsigned char *init_vector){
	gcry_error_t err;

	// 256-bit AES using cipher-block chaining; with ciphertext stealing, no manual padding is required
	err = gcry_cipher_open(handle,GCRY_CIPHER_AES256,GCRY_CIPHER_MODE_CBC,GCRY_CIPHER_CBC_CTS);

	if(err){
		fprintf(stderr,"cipher_open: %s/%s\n",gcry_strsource(err),gcry_strerror(err));
		return 1;
	}

	err = gcry_cipher_setkey(*handle,key,AES256_KEYSIZ);
	if(err){
		fprintf(stderr,"cipher_setke: %s/%s\n",gcry_strsource(err), gcry_strerror(err));
		gcry_cipher_close(*handle);
		return 1;
	}

	err = gcry_cipher_setiv(*handle,init_vector,AES256_BLOCKSIZ);
	if(err){
		fprintf(stderr,"cipher_setiv: %s/%s\n",gcry_strsource(err), gcry_strerror(err));
		gcry_cipher_close(*handle);
		return 1;
	}

	return 0;
}

int encrypt_file(char *infile, char *outfile, char *password){
	unsigned char init_vector[AES256_BLOCKSIZ],
			kdf_salt[KDF_SALTSIZ],
			kdf_key[KDF_KEYSIZ],
			aes_key[AES256_KEYSIZ],
			hmac_key[HMAC_KEYSIZ],
			*hmac,
			*packed_data,
			*ciphertext,
			*text;

	size_t blocks_required, text_len, packed_data_len, hmac_len;
	gcry_cipher_hd_t handle;
	gcry_mac_hd_t mac;
	gcry_error_t err;

	// fetch text to be encrypted
	if(!(text_len = read_file_info_buf(infile,&text))){
		return 1;
	}

	// find number of blocks required for data
	blocks_required = text_len / AES256_BLOCKSIZ;
	if(text_len % AES256_BLOCKSIZ != 0){
		blocks_required++;
	}

	// generate 128 byte salt in preperation for key derivation
	gcry_create_nonce(kdf_salt,KDF_SALTSIZ);

	// key derivation: PBKDF2 using SHA512 with 128 byte salt over 10 iterations into a 65 byte key
	err = gcry_kdf_derive(password,strlen(password),GCRY_KDF_PBKDF2,GCRY_MD_SHA512,kdf_salt,KDF_SALTSIZ,KDF_ITERATIONS,KDF_KEYSIZ,kdf_key);

	if(err){
		fprintf(stderr,"kdf_derive: %s/%s\n",gcry_strsource(err), gcry_strerror(err));
		free(text);
		return 1;
	}

	// copy the first 32 bytes of kdf_key into aes_key
	memcpy(aes_key,kdf_key,AES256_KEYSIZ);

	// copy last 32 bytes of kdf_key into hmac_key
	memcpy(hmac_key,&(kdf_key[AES256_KEYSIZ]),HMAC_KEYSIZ);

	// generate the initialization vector
	gcry_create_nonce(init_vector,AES256_BLOCKSIZ);

	// begin encryption
	if(init_cipher(&handle,aes_key,init_vector)){
		free(text);
		return 1;
	}

	// make new buffer of size blocks_required * AES256_BLOCKSIZ for in-place encryption
	ciphertext = malloc(blocks_required * AES256_BLOCKSIZ);
	if(ciphertext == NULL){
		fprintf(stderr,"Error: unable to allocate memory for the ciphertext\n");
		cleanup(handle,NULL,text,NULL,NULL);
		return 1;
	}
	memcpy(ciphertext,text,blocks_required * AES256_BLOCKSIZ);
	free(text);

	// encyption is performed in-place
	err = gcry_cipher_encrypt(handle,ciphertext,AES256_BLOCKSIZ * blocks_required,NULL,0);
	if(err){
		fprintf(stderr,"cipher_encrypt: %s/%s\n",gcry_strsource(err), gcry_strerror(err));
		cleanup(handle,NULL,ciphertext,NULL,NULL);
		return 1;
	}


	// compute and allocate space required for packed data
	hmac_len = gcry_mac_get_algo_maclen(GCRY_MAC_HMAC_SHA512);
	packed_data_len = KDF_SALTSIZ + AES256_BLOCKSIZ + (AES256_BLOCKSIZ * blocks_required) + hmac_len;
	packed_data = malloc(packed_data_len);
	if(packed_data == NULL){
		fprintf(stderr,"Unable to allocate memory for packed data\n");
		cleanup(handle,NULL,ciphertext,NULL,NULL);
		return 1;
	}

	// pack data before writing: salt::IV::ciphertext::HMAC where "::" denotes concatenation
	memcpy(packed_data,kdf_salt,KDF_SALTSIZ);
	memcpy(&(packed_data[KDF_SALTSIZ]),init_vector,AES256_BLOCKSIZ);
	memcpy(&(packed_data[KDF_SALTSIZ + AES256_BLOCKSIZ]),ciphertext,AES256_BLOCKSIZ*blocks_required);

	// begin HMAC computation on encrypted/packed data
	hmac = malloc(hmac_len);
	if(hmac == NULL){
		fprintf(stderr,"Error: unable to allocate memory for the HMAC\n");
		cleanup(handle,NULL,ciphertext,packed_data,NULL);
		return 1;
	}

	err = gcry_mac_open(&mac,GCRY_MAC_HMAC_SHA512,0,NULL);
	if(err){
		fprintf(stderr,"mac_open during encryption: %s/%s\n",gcry_strsource(err), gcry_strerror(err));
		cleanup(handle,NULL,ciphertext,packed_data,hmac);
		return 1;
	}

	err = gcry_mac_setkey(mac,hmac_key,HMAC_KEYSIZ);
	if(err){
	}
}


int main(int agrc, char *argv[]){
}
