XINDEX ;ISC/REL,GFT,GRK,RWF - INDEX & CROSS-REFERENCE ;08/04/08  13:19
 ;;7.3;TOOLKIT;**20,27,48,61,66,68,110,121,128**;Apr 25, 1995;Build 1
 ; Per VHA Directive 2004-038, this routine should not be modified.
 G ^XINDX6
SEP F I=1:1 S CH=$E(LIN,I) D QUOTE:CH=Q Q:" "[CH
 S ARG=$E(LIN,1,I-1) S:CH=" " I=I+1 S LIN=$E(LIN,I,999) Q
QUOTE F I=I+1:1 S CH=$E(LIN,I) Q:CH=""!(CH=Q)
 Q:CH]""  S ERR=6 G ^XINDX1
ALIVE ;enter here from taskman
 D SETUP^XINDX7 ;Get ready to process
A2 S RTN=$O(^UTILITY($J,RTN)) G ^XINDX5:RTN=""
 S INDLC=(RTN?1"|"1.4L.NP) D LOAD:'INDLC
 I $D(ZTQUEUED),$$S^%ZTLOAD S RTN="~",IND("QUIT")=1,ZTSTOP=1 G A2
 I 'INDDS,INDLC W !!?10,"Data Dictionaries",! S INDDS=1
 D BEG
 G A2
 ;
LOAD S X=RTN,XCNP=0,DIF="^UTILITY("_$J_",1,RTN,0," X ^%ZOSF("TEST") Q:'$T  X ^%ZOSF("LOAD") S ^UTILITY($J,1,RTN,0,0)=XCNP-1
 I $D(^UTILITY($J,1,RTN,0,0)) S ^UTILITY($J,1,RTN,"RSUM")="B"_$$SUMB^XPDRSUM($NA(^UTILITY($J,1,RTN,0)))
 Q
BEG ;
 S %=INDLC*5 W:$X+10+%>IOM ! W RTN,$J("",10+%-$L(RTN))
 S (IND("DO"),IND("SZT"),IND("SZC"),LABO)=0,LC=$G(^UTILITY($J,1,RTN,0,0))
 I LC="" W !,">>>Routine '",RTN,"' not found <<<",! Q
 S TXT="",LAB=$P(^UTILITY($J,1,RTN,0,1,0)," ") I RTN'=$P(LAB,"(") D E^XINDX1(17)
 I 'INDLC,LAB["(" D E^XINDX1(55) S LAB=$P(LAB,"(")
 ;if M routine(not compiled template or DD) and has more than 2 lines, check lines 1 & 2
 I 'INDLC,LC>2 D
 . N LABO S LABO=1
 . S LIN=$G(^UTILITY($J,1,RTN,0,1,0)),TXT=1
 . ;check 1st line (site/dev - ) patch 128
 . I $P(LIN,";",2,4)'?.E1"/".E.1"-".E D E^XINDX1(62)
 . S LIN=$G(^UTILITY($J,1,RTN,0,2,0)),TXT=2
 . ;check 2nd line (;;nn.nn[TV]nn;package;.anything)
 . I $P(LIN,";",3,99)'?1.2N1"."1.2N.1(1"T",1"V").2N1";"1A.AP1";".E D E^XINDX1(44) ;patch 121
 . I $L(INP(11)) X INP(11) ;Version number check
 . I $L(INP(12)) X INP(12) ;Patch number check
B5 F TXT=1:1:LC S LIN=^UTILITY($J,1,RTN,0,TXT,0),LN=$L(LIN),IND("SZT")=IND("SZT")+LN+2 D LN,ST ;Process Line
 S LAB="",LABO=0,TXT=0,^UTILITY($J,1,RTN,0)=IND("SZT")_"^"_LC_"^"_IND("SZC")
 I IND("SZT")>INP("MAX"),'INDLC S ERR=35,ERR(1)=IND("SZT") D ^XINDX1
 I IND("SZT")-IND("SZC")>INP("CMAX"),'INDLC S ERR=58,ERR(1)=IND("SZT")-IND("SZC") D ^XINDX1
 D POSTRTN
 Q
 ;Proccess one line, LN = Length, LIN = Line.
LN K V S (ARG,GRB,IND("COM"),IND("DOL"),IND("F"))="",X=$P(LIN," ")
 I '$L(X) S LABO=LABO+1 G CD
 S (IND("COM"),LAB)=$P(X,"("),ARG=$P($P(X,"(",2),")"),LABO=0,IND("PP")=X?1.8E1"(".E1")"
 D:$L(ARG) NE^XINDX3 ;Process formal parameters as New list.
 I 'INDLC,'$$VT^XINDX2(LAB) D E^XINDX1($S(LAB=$$CASE^XINDX52(LAB):37,1:55)) ;Check for bad labels
 I $D(^UTILITY($J,1,RTN,"T",LAB)) D E^XINDX1(15) G CD ;DUP label
 S ^UTILITY($J,1,RTN,"T",LAB)=""
CD I LN>245 D:'(LN=246&($E(RTN,1,3)="|dd")) E^XINDX1(19) ;patch 119
 D:LIN'?1.ANP E^XINDX1(18)
 S LIN=$P(LIN," ",2,999),IND("LCC")=1
 I LIN="" D E^XINDX1(42) Q  ;Blank line ;p110
 S I=0 ;Watch the scope of I, counts dots
 I " ."[$E(LIN) D  S X=$L($E(LIN,1,I),".")-1,LIN=$E(LIN,I,999)
 . F I=1:1:245 Q:". "'[$E(LIN,I)
 . Q
 ;check dots against Do level IND("DO"), IND("DOL")=dot level
 D:'I&$G(IND("DO1")) E^XINDX1(51) S IND("DO1")=0 S:'I IND("DO")=0
 I I D:X>IND("DO") E^XINDX1(51) S (IND("DO"),IND("DOL"))=X
 ;Count Comment lines, skip ;; lines
 I $E(LIN)=";",$E(LIN,2)'=";" S IND("SZC")=IND("SZC")+$L(LIN) ;p110
 ;Process commands on line.
EE I LIN="" D ^XINDX2 Q
 S COM=$E(LIN),GK="",ARG=""
 I COM=";" S LIN="" G EE ;p110
 I COM=" " S ERR=$S(LIN?1." ":13,1:0),LIN=$S(ERR:"",1:$E(LIN,2,999)) D:ERR ^XINDX1 G EE
 D SEP
 S CM=$P(ARG,":",1),POST=$P(ARG,":",2,999),IND("COM")=IND("COM")_$C(9)_COM,ERR=48
 D:ARG[":"&(POST']"") ^XINDX1 S:POST]"" GRB=GRB_$C(9)_POST,IND("COM")=IND("COM")_":"
 ;SAC now allows lowercase commands
 I CM?.E1L.E S CM=$$CASE^XINDX52(CM),COM=$E(CM) ;I IND("LCC") S IND("LCC")=0 D E^XINDX1(47)
 I CM="" D E^XINDX1(21) G EE ;Missing command
 S CX=$G(IND("CMD",CM)) I CX="" D  G:CX="" EE
 . I $E(CM)="Z" S CX="^Z" Q  ;Proccess Z commands
 . D E^XINDX1(1) S LIN="" Q
 S CX=$P(CX,"^",2,9)
 D SEP I '$L(LIN),CH=" " D E^XINDX1(13) ;trailing space
 I ARG="","CGJMORSUWX"[COM S ERR=49 G ^XINDX1
 I CX>0 D E^XINDX1(CX) S CX=""
 D:$L(CX) @CX S:ARG'="" GRB=GRB_$C(9)_ARG G EE
B S ERR=25 G ^XINDX1
C S ERR=29 G ^XINDX1
D G DG1^XINDX4
E Q:ARG=""  S ERR=7 G ^XINDX1
F G:ARG]"" FR^XINDX4 S IND("F")=1 Q
G G DG^XINDX4
H Q:ARG'=""  S ERR=32 G ^XINDX1
J S ERR=36,ARG="" G ^XINDX1
K S ERR=$S(ARG?1"(".E:22,ARG?." ":23,1:0) D:ERR ^XINDX1
 G KL^XINDX3
L G LO^XINDX4
M G S^XINDX3
N G NE^XINDX3
O S ERR=34 D ^XINDX1,O^XINDX3 Q
Q Q:ARG=""  G Q^XINDX4
R S RDTIME=0 G RD^XINDX3
S G S^XINDX3
TR Q  ;What to process. p110
U S ARG=$P(ARG,":") Q
V S ARG="",ERR=20 G ^XINDX1
W G WR^XINDX4
X G XE^XINDX4
Z S ERR=2 D ^XINDX1 G ZC^XINDX4
 ;
 ;Save off items from line.
ST S R=LAB_$S(LABO:"+"_LABO,1:"")
 ;Local variable, Global, Marked Items, Naked global, Internal ref, eXternal ref., Tag ref.
 S LOC="" F  S LOC=$O(V(LOC)),S="" Q:LOC=""  F  S S=$O(V(LOC,S)) Q:S=""  D SET
 S ^UTILITY($J,1,RTN,"COM",TXT)=IND("COM")
 Q
 ;
SET I V(LOC,S)]"" F %="!","~" I V(LOC,S)[%,$G(^UTILITY($J,1,RTN,LOC,S))'[% S ^(S)=$G(^(S))_%
 S %=0
SE2 S ARG=$G(^UTILITY($J,1,RTN,LOC,S,%)) I $L(ARG)>230 S %=%+1 G SE2
 S ^UTILITY($J,1,RTN,LOC,S,%)=ARG_R_V(LOC,S)_","
 Q
 ;
POSTRTN ;Do more overall checking
 N V,E,T,T1,T2
 S T="" ;Check for missing Labels
 F  S T=$O(^UTILITY($J,1,RTN,"I",T)),T2=T Q:T=""  S T1=$G(^(T,0)) D
 . Q:$E(T2,1,2)="@("
 . S:$E(T2,1,2)="$$" T2=$E(T2,3,99)
 . I T2]"",'$D(^UTILITY($J,1,RTN,"T",$P(T2,"+",1))) D
 . . F I=1:1:$L(T1,",")-1 S LAB=$P(T1,",",I),LABO=+$P(LAB,"+",2),LAB=$P(LAB,"+"),E=14,E(1)=T D E^XINDX1(.E)
 . . Q
 . Q
 S LAB="",LABO=0 ;Check for valid label names
 I 'INDLC F  S LAB=$O(^UTILITY($J,1,RTN,"T",LAB)) Q:LAB=""  D
 . I '$$VA^XINDX2(LAB) D E^XINDX1(55) Q
 . D:'$$VT^XINDX2(LAB) E^XINDX1(37)
 . Q
 S LAB="",LABO=0 ;Check for valid variable names.
 F  S LAB=$O(^UTILITY($J,1,RTN,"L",LAB)) Q:LAB=""  D
 . D VLNF^XINDX3($P(LAB,"("))
 . Q
 Q
 ;
QUICK ;Quick, Just get a routine an print the results
 D QUICK^XINDX6()
 Q
