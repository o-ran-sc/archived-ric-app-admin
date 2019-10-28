/*
 * Generated by asn1c-0.9.29 n1 (http://lionet.info/asn1c)
 * From ASN.1 module "X2AP-IEs"
 * 	found in "../../asn_defs/asn1/x2ap-15-04.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-OER`
 */

#ifndef	_X2N_EN_DC_ResourceConfiguration_H_
#define	_X2N_EN_DC_ResourceConfiguration_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum X2N_EN_DC_ResourceConfiguration__pDCPatSgNB {
	X2N_EN_DC_ResourceConfiguration__pDCPatSgNB_present	= 0,
	X2N_EN_DC_ResourceConfiguration__pDCPatSgNB_not_present	= 1
	/*
	 * Enumeration is extensible
	 */
} e_X2N_EN_DC_ResourceConfiguration__pDCPatSgNB;
typedef enum X2N_EN_DC_ResourceConfiguration__mCGresources {
	X2N_EN_DC_ResourceConfiguration__mCGresources_present	= 0,
	X2N_EN_DC_ResourceConfiguration__mCGresources_not_present	= 1
	/*
	 * Enumeration is extensible
	 */
} e_X2N_EN_DC_ResourceConfiguration__mCGresources;
typedef enum X2N_EN_DC_ResourceConfiguration__sCGresources {
	X2N_EN_DC_ResourceConfiguration__sCGresources_present	= 0,
	X2N_EN_DC_ResourceConfiguration__sCGresources_not_present	= 1
	/*
	 * Enumeration is extensible
	 */
} e_X2N_EN_DC_ResourceConfiguration__sCGresources;

/* Forward declarations */
struct X2N_ProtocolExtensionContainer;

/* X2N_EN-DC-ResourceConfiguration */
typedef struct X2N_EN_DC_ResourceConfiguration {
	long	 pDCPatSgNB;
	long	 mCGresources;
	long	 sCGresources;
	struct X2N_ProtocolExtensionContainer	*iE_Extensions;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} X2N_EN_DC_ResourceConfiguration_t;

/* Implementation */
/* extern asn_TYPE_descriptor_t asn_DEF_X2N_pDCPatSgNB_2;	// (Use -fall-defs-global to expose) */
/* extern asn_TYPE_descriptor_t asn_DEF_X2N_mCGresources_6;	// (Use -fall-defs-global to expose) */
/* extern asn_TYPE_descriptor_t asn_DEF_X2N_sCGresources_10;	// (Use -fall-defs-global to expose) */
extern asn_TYPE_descriptor_t asn_DEF_X2N_EN_DC_ResourceConfiguration;
extern asn_SEQUENCE_specifics_t asn_SPC_X2N_EN_DC_ResourceConfiguration_specs_1;
extern asn_TYPE_member_t asn_MBR_X2N_EN_DC_ResourceConfiguration_1[4];

#ifdef __cplusplus
}
#endif

#endif	/* _X2N_EN_DC_ResourceConfiguration_H_ */
#include <asn_internal.h>
