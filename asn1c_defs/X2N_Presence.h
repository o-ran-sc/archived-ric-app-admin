/*
 * Generated by asn1c-0.9.29 n1 (http://lionet.info/asn1c)
 * From ASN.1 module "X2AP-CommonDataTypes"
 * 	found in "../../asn_defs/asn1/x2ap-15-04.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-OER`
 */

#ifndef	_X2N_Presence_H_
#define	_X2N_Presence_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum X2N_Presence {
	X2N_Presence_optional	= 0,
	X2N_Presence_conditional	= 1,
	X2N_Presence_mandatory	= 2
} e_X2N_Presence;

/* X2N_Presence */
typedef long	 X2N_Presence_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_X2N_Presence_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_X2N_Presence;
extern const asn_INTEGER_specifics_t asn_SPC_Presence_specs_1;
asn_struct_free_f Presence_free;
asn_struct_print_f Presence_print;
asn_constr_check_f Presence_constraint;
ber_type_decoder_f Presence_decode_ber;
der_type_encoder_f Presence_encode_der;
xer_type_decoder_f Presence_decode_xer;
xer_type_encoder_f Presence_encode_xer;
per_type_decoder_f Presence_decode_uper;
per_type_encoder_f Presence_encode_uper;
per_type_decoder_f Presence_decode_aper;
per_type_encoder_f Presence_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _X2N_Presence_H_ */
#include <asn_internal.h>
