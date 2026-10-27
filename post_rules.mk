RGB_MATRIX_CUSTOM_USER = yes
include keyboards/monsgeek/m1_v5/m1_v5_us/rgb_record/rgb_record.mk
include keyboards/monsgeek/m1_v5/m1_v5_us/wls/wls.mk
include keyboards/linker/wireless/wireless.mk


# OpenRGB (VialRGB protocol) shares VIA's raw HID channel, so it needs VIA.
ifeq ($(strip $(VIA_ENABLE)), yes)
    SRC += vialrgb/vialrgb.c
endif
