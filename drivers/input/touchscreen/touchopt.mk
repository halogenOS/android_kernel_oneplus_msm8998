
ccflags-y ?=
ccflags-y += -O3 \
    -fno-defer-pop \
    -ffp-contract=fast \
    -fgcse-sm \
    -fgcse-las \
    -fgcse-after-reload \
    -fno-signed-zeros \
    -fno-trapping-math \
    -fno-associative-math \
    -fno-reciprocal-math \
    -ffinite-math-only
ifneq ($(GCC_VERSION), 0409)
ccflags-y += \
    -mtune=cortex-a73 \
    -mcpu=cortex-a73
endif
