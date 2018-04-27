#ifndef FS_ECDH_H
#define FS_ECDH_H

#include <crypto/kpp.h>
#include <crypto/ecdh.h>
#if DEFINE_F2FS_FS_SDP_ENCRYPTION
/**
 * get_file_pubkey_shared_secret() - Use ECDH to generate file public key
 * and shared secret.
 *
 * @cuive_id:          Curve id, only ECC_CURVE_NIST_P256 now
 * @dev_pub_key:       Device public key
 * @dev_pub_key_len:   The length of @dev_pub_key in bytes
 * @file_pub_key[out]: File public key to be generated
 * @file_pub_key_len:  The length of @file_pub_key in bytes
 * @shared secret[out]:Compute shared secret with file private key
 * and device public key
 * @shared_secret_len: The length of @shared_secret in bytes
 *
 * Return: 0 for success, error code in case of error.
 *         The contents of @file_pub_key and @shared_secret are
 *         undefined in case of error.
 */
int get_file_pubkey_shared_secret(unsigned int curve_id,
	const u8     *dev_pub_key,
	unsigned int dev_pub_key_len,
	u8           *file_pub_key,
	unsigned int file_pub_key_len,
	u8           *shared_secret,
	unsigned int shared_secret_len);

/**
 * get_shared_secret() - Use ECDH to generate shared secret.
 *
 * @cuive_id:          Curve id, only ECC_CURVE_NIST_P256 now
 * @dev_privkey:       Device private key
 * @dev_privkey_len:   The length of @dev_privkey in bytes
 * @file_pub_key:      File public key
 * @file_pub_key_len:  The length of @file_pub_key in bytes
 * @shared secret[out]:Compute shared secret with file public key and device
 *                     private key
 * @shared_secret_len: The length of @shared_secret in bytes
 *
 * Return: 0 for success, error code in case of error.
 *         The content of @shared_secret is undefined in case of error.
 */
int get_shared_secret(unsigned int curve_id,
	const u8     *dev_privkey,
	unsigned int dev_privkey_len,
	const u8     *file_pub_key,
	unsigned int file_pub_key_len,
	u8           *shared_secret,
	unsigned int shared_secret_len);

#endif
#endif
