#ifndef DROPBEAR_LOCALOPTIONS_H
#define  DROPBEAR_LOCALOPTIONS_H

#include <sys/select.h>

#define DSS_PRIV_FILENAME "/local/dropbear_dss_host_key"
#define RSA_PRIV_FILENAME "/local/dropbear_rsa_host_key"
#define ECDSA_PRIV_FILENAME "/local/dropbear_ecdsa_host_key"
#define DROPBEAR_USE_PASSWORD_ENV 1

#ifdef ENABLE_PS_LOGIN_SERVICE
#include <phoenix.h>
#endif

#endif
