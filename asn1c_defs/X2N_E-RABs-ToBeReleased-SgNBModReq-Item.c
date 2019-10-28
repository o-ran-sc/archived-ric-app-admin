/*
 * Generated by asn1c-0.9.29 n1 (http://lionet.info/asn1c)
 * From ASN.1 module "X2AP-PDU-Contents"
 * 	found in "../../asn_defs/asn1/x2ap-15-04.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-OER`
 */

#include "X2N_E-RABs-ToBeReleased-SgNBModReq-Item.h"

#include "X2N_ProtocolExtensionContainer.h"
#include "X2N_E-RABs-ToBeReleased-SgNBModReq-Item-SgNBPDCPpresent.h"
#include "X2N_E-RABs-ToBeReleased-SgNBModReq-Item-SgNBPDCPnotpresent.h"
static asn_per_constraints_t asn_PER_type_X2N_resource_configuration_constr_4 CC_NOTUSED = {
	{ APC_CONSTRAINED | APC_EXTENSIBLE,  1,  1,  0,  1 }	/* (0..1,...) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_TYPE_member_t asn_MBR_X2N_resource_configuration_4[] = {
	{ ATF_POINTER, 0, offsetof(struct X2N_E_RABs_ToBeReleased_SgNBModReq_Item__resource_configuration, choice.sgNBPDCPpresent),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_X2N_E_RABs_ToBeReleased_SgNBModReq_Item_SgNBPDCPpresent,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"sgNBPDCPpresent"
		},
	{ ATF_POINTER, 0, offsetof(struct X2N_E_RABs_ToBeReleased_SgNBModReq_Item__resource_configuration, choice.sgNBPDCPnotpresent),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_X2N_E_RABs_ToBeReleased_SgNBModReq_Item_SgNBPDCPnotpresent,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"sgNBPDCPnotpresent"
		},
};
static const asn_TYPE_tag2member_t asn_MAP_X2N_resource_configuration_tag2el_4[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* sgNBPDCPpresent */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* sgNBPDCPnotpresent */
};
static asn_CHOICE_specifics_t asn_SPC_X2N_resource_configuration_specs_4 = {
	sizeof(struct X2N_E_RABs_ToBeReleased_SgNBModReq_Item__resource_configuration),
	offsetof(struct X2N_E_RABs_ToBeReleased_SgNBModReq_Item__resource_configuration, _asn_ctx),
	offsetof(struct X2N_E_RABs_ToBeReleased_SgNBModReq_Item__resource_configuration, present),
	sizeof(((struct X2N_E_RABs_ToBeReleased_SgNBModReq_Item__resource_configuration *)0)->present),
	asn_MAP_X2N_resource_configuration_tag2el_4,
	2,	/* Count of tags in the map */
	0, 0,
	2	/* Extensions start */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_X2N_resource_configuration_4 = {
	"resource-configuration",
	"resource-configuration",
	&asn_OP_CHOICE,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	{ 0, &asn_PER_type_X2N_resource_configuration_constr_4, CHOICE_constraint },
	asn_MBR_X2N_resource_configuration_4,
	2,	/* Elements count */
	&asn_SPC_X2N_resource_configuration_specs_4	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_X2N_E_RABs_ToBeReleased_SgNBModReq_Item_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct X2N_E_RABs_ToBeReleased_SgNBModReq_Item, e_RAB_ID),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_X2N_E_RAB_ID,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"e-RAB-ID"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct X2N_E_RABs_ToBeReleased_SgNBModReq_Item, en_DC_ResourceConfiguration),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_X2N_EN_DC_ResourceConfiguration,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"en-DC-ResourceConfiguration"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct X2N_E_RABs_ToBeReleased_SgNBModReq_Item, resource_configuration),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_X2N_resource_configuration_4,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"resource-configuration"
		},
	{ ATF_POINTER, 1, offsetof(struct X2N_E_RABs_ToBeReleased_SgNBModReq_Item, iE_Extensions),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_X2N_ProtocolExtensionContainer_8231P59,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"iE-Extensions"
		},
};
static const int asn_MAP_X2N_E_RABs_ToBeReleased_SgNBModReq_Item_oms_1[] = { 3 };
static const ber_tlv_tag_t asn_DEF_X2N_E_RABs_ToBeReleased_SgNBModReq_Item_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_X2N_E_RABs_ToBeReleased_SgNBModReq_Item_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* e-RAB-ID */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* en-DC-ResourceConfiguration */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* resource-configuration */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 } /* iE-Extensions */
};
static asn_SEQUENCE_specifics_t asn_SPC_X2N_E_RABs_ToBeReleased_SgNBModReq_Item_specs_1 = {
	sizeof(struct X2N_E_RABs_ToBeReleased_SgNBModReq_Item),
	offsetof(struct X2N_E_RABs_ToBeReleased_SgNBModReq_Item, _asn_ctx),
	asn_MAP_X2N_E_RABs_ToBeReleased_SgNBModReq_Item_tag2el_1,
	4,	/* Count of tags in the map */
	asn_MAP_X2N_E_RABs_ToBeReleased_SgNBModReq_Item_oms_1,	/* Optional members */
	1, 0,	/* Root/Additions */
	4,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_X2N_E_RABs_ToBeReleased_SgNBModReq_Item = {
	"E-RABs-ToBeReleased-SgNBModReq-Item",
	"E-RABs-ToBeReleased-SgNBModReq-Item",
	&asn_OP_SEQUENCE,
	asn_DEF_X2N_E_RABs_ToBeReleased_SgNBModReq_Item_tags_1,
	sizeof(asn_DEF_X2N_E_RABs_ToBeReleased_SgNBModReq_Item_tags_1)
		/sizeof(asn_DEF_X2N_E_RABs_ToBeReleased_SgNBModReq_Item_tags_1[0]), /* 1 */
	asn_DEF_X2N_E_RABs_ToBeReleased_SgNBModReq_Item_tags_1,	/* Same as above */
	sizeof(asn_DEF_X2N_E_RABs_ToBeReleased_SgNBModReq_Item_tags_1)
		/sizeof(asn_DEF_X2N_E_RABs_ToBeReleased_SgNBModReq_Item_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_X2N_E_RABs_ToBeReleased_SgNBModReq_Item_1,
	4,	/* Elements count */
	&asn_SPC_X2N_E_RABs_ToBeReleased_SgNBModReq_Item_specs_1	/* Additional specs */
};

