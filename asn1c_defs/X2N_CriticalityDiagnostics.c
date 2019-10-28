/*
 * Generated by asn1c-0.9.29 n1 (http://lionet.info/asn1c)
 * From ASN.1 module "X2AP-IEs"
 * 	found in "../../asn_defs/asn1/x2ap-15-04.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-OER`
 */

#include "X2N_CriticalityDiagnostics.h"

#include "X2N_CriticalityDiagnostics-IE-List.h"
#include "X2N_ProtocolExtensionContainer.h"
static asn_TYPE_member_t asn_MBR_X2N_CriticalityDiagnostics_1[] = {
	{ ATF_POINTER, 5, offsetof(struct X2N_CriticalityDiagnostics, procedureCode),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_X2N_ProcedureCode,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"procedureCode"
		},
	{ ATF_POINTER, 4, offsetof(struct X2N_CriticalityDiagnostics, triggeringMessage),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_X2N_TriggeringMessage,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"triggeringMessage"
		},
	{ ATF_POINTER, 3, offsetof(struct X2N_CriticalityDiagnostics, procedureCriticality),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_X2N_Criticality,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"procedureCriticality"
		},
	{ ATF_POINTER, 2, offsetof(struct X2N_CriticalityDiagnostics, iEsCriticalityDiagnostics),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_X2N_CriticalityDiagnostics_IE_List,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"iEsCriticalityDiagnostics"
		},
	{ ATF_POINTER, 1, offsetof(struct X2N_CriticalityDiagnostics, iE_Extensions),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_X2N_ProtocolExtensionContainer_8231P126,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"iE-Extensions"
		},
};
static const int asn_MAP_X2N_CriticalityDiagnostics_oms_1[] = { 0, 1, 2, 3, 4 };
static const ber_tlv_tag_t asn_DEF_X2N_CriticalityDiagnostics_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_X2N_CriticalityDiagnostics_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* procedureCode */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* triggeringMessage */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* procedureCriticality */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* iEsCriticalityDiagnostics */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 } /* iE-Extensions */
};
static asn_SEQUENCE_specifics_t asn_SPC_X2N_CriticalityDiagnostics_specs_1 = {
	sizeof(struct X2N_CriticalityDiagnostics),
	offsetof(struct X2N_CriticalityDiagnostics, _asn_ctx),
	asn_MAP_X2N_CriticalityDiagnostics_tag2el_1,
	5,	/* Count of tags in the map */
	asn_MAP_X2N_CriticalityDiagnostics_oms_1,	/* Optional members */
	5, 0,	/* Root/Additions */
	5,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_X2N_CriticalityDiagnostics = {
	"CriticalityDiagnostics",
	"CriticalityDiagnostics",
	&asn_OP_SEQUENCE,
	asn_DEF_X2N_CriticalityDiagnostics_tags_1,
	sizeof(asn_DEF_X2N_CriticalityDiagnostics_tags_1)
		/sizeof(asn_DEF_X2N_CriticalityDiagnostics_tags_1[0]), /* 1 */
	asn_DEF_X2N_CriticalityDiagnostics_tags_1,	/* Same as above */
	sizeof(asn_DEF_X2N_CriticalityDiagnostics_tags_1)
		/sizeof(asn_DEF_X2N_CriticalityDiagnostics_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_X2N_CriticalityDiagnostics_1,
	5,	/* Elements count */
	&asn_SPC_X2N_CriticalityDiagnostics_specs_1	/* Additional specs */
};

