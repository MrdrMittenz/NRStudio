#pragma once
struct NRDirectSpec { const char* key; const char* name; unsigned bytes, blockY; };
static constexpr NRDirectSpec NR_DIRECT_SPECS[]={
 {"post","cc_tinlayout_fused_post_block_swin_1h_32_fp8",184,1},
 {"swin1","cc_tinlayout_fused_swin_1h_32_1_chained_fp8",96,1},
 {"pre","cc_tinlayout_fused_pre_block_swin_1h_32_1_ds_fp8",264,1},
 {"swin8","cc_tinlayout_fused_swin_8h_256_8_chained_fp8",88,8},
};
