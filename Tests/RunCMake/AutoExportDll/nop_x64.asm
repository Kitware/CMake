IFDEF RAX
ELSE
.MODEL FLAT,C
ENDIF

.CODE

public justnop
justnop PROC
  ret
justnop ENDP

END
