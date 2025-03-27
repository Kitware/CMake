#if !defined(__IAR_SYSTEMS_ASM__)
#error This source file should be assembled by the IAR Assembler.
#endif

#if !defined(__A430__)      && \
    !(((__TID__ >> 8) & 0x7F) == 32) && \
    !(((__TID__ >> 8) & 0x7F) == 90) && \
    !defined(__IASMARM__)   && \
    !defined(__IASMRH850__) && \
    !defined(__IASMRISCV__) && \
    !defined(__IASMRL78__)  && \
    !defined(__IASMRX__)    && \
    !defined(__IASMSTM8__)
#error Unable to detect a supported target architecture.
#endif
        NAME    main

        PUBLIC  main
        PUBLIC  __iar_program_start
        PUBLIC  __program_start

#if defined(__A430__)
        ORG     0FFFEh
        DC16    init
        RSEG    CSTACK
        RSEG    CODE
init:   MOV     #SFE(CSTACK), SP
        RSEG    CODE
#elif (((__TID__ >> 8) & 0x7F) == 32)
        ORG     0FFFEh                 ; __A8051__
        DC16    main
        RSEG    RCODE
#elif (((__TID__ >> 8) & 0x7F) == 90)
        ORG     $0                     ; __AAVR__
        RJMP    main
#elif defined(__IASMARM__)   || \
      defined(__IASMRH850__) || \
      defined(__IASMRISCV__) || \
      defined(__IASMRL78__)  || \
      defined(__IASMRX__)
        EXTERN  __iar_static_base$$GPREL
        SECTION CSTACK:DATA:NOROOT(4)
        SECTION `.cstartup`:CODE(2)
        CODE
#elif defined(__IASMSTM8__)
        EXTERN  CSTACK$$Limit
        SECTION `.near_func.text`:CODE:NOROOT(0)
#endif

__program_start:
__iar_program_start:
main:   NOP
        END
