#include "utils.h" 

int main() {
    // Good enough order finding
    srand(time(NULL));
    char vstup[] = "!M!Q!X+CGU!EVV+GKNW!K+K!JM+MNNTU!AGRU+CKPV!CDGW+HIS!TUY+EO!U+DW!Z+GGLWZ!AIS+GHLW!EIKY+CKUV!X+DIYY!UVX+Q!ARR+CKMMQ!CUXY+MT!ADDH+IJMOS!BCMX+DLLUW!HKOXZ+OYZ!AGGO+EJ!G+K!AFGUW+JSY!CNT+Q!CDHLX+BKMUZ!DO+AEI!LM+HOR!T+MTY!JRY+HPPUY!CQTZ+DGHZ!EKNS+GLTUZ!FJSVZ+AFNUY!IJWX+B!FHQZ+INX!BMX+BIR!DDDLW+G!DT+FHISV!DKMMN+I!DDJ+P!GMQZ+DLS!EEHKT+BCLNP!BU+BCNX!BJJMS+Z!GRZ+FHSZ!IL+DHJR!U+DFJW!BCJTZ+BBEFG!OOPTU+DEHOY!R+JLR!W+DIKSU!V+HT!MR+HI!ABO+AZ!BSTX+EUYZ!CKLNQ+BG!EM+IJKQT!CQS+HI!HLN+DJM!JT+HVWX!PR+XZ!BW+BCW!WZ+P!BX+DFS!GIQR+IKOT!V+IJ!AT+F!BN+EQSVX!DV+DFMQR!Z+OQSU!K+GPST!B+KSV!KKLP+OSW!CCCCH+CQ!N+T!QS+OQ!JNVXY+U!EENYY+DHNVW!ES+DQ!ABHIM+BEGH!DEJXX+EHMW!AEQVZ+CEHN!CO+IU!CGQS+CK!ACELV+CR!PXY+BBFGN!AEWX+DP!FOW+DDJK!Z+EFJKV!IIJ+BKZ!QR+W!MM+BW!AO+ABIL!LRZ+CD!BRXYZ+PQTW!KU+HM!U+EQUW!KR+NRY!GJW+FQ!CDILO+MQVY+FLNTX+LW!GOSUX+EVV!CK+NY!AEFV+A!BBCLU+AU!QWX+FNNSS!OOUX+HS!BY+BCTW!EIPU+BSST!L+JLV!L+Z!BHP+D!L+IX!HKPUY+BFOOX!OOOQU+IQ!M+EGR!CFHY+INY!CJKOU+HRX!BOP+AX!AEIVV+NZ!CGJR+KLO!UY+HS!BCFG+AQ!QQTWZ+AHMOX!";
    char poradie[] = "EKUVCHIJNLMQRTABDFGOPSXYZW";

    BDD* bdd = BDD_create_with_best_order(vstup, poradie);
    printf("Good enough velkost: %d\n", bdd->size);


    //Testy
    bdd = BDD_create("A!B!CD+!CDBA+!A!B!C!D+A!B!C!D+ABC!D", "ABCD");

    if (BDD_use(bdd, "0000")[0] != '1')
        printf("error, for A=0, B=0, C=0, D=0 result should be 1.\n");
    if (BDD_use(bdd, "1000")[0] != '1')
        printf("error, for A=1, B=0, C=0, D=0 result should be 1.\n");
    if (BDD_use(bdd, "1001")[0] != '1')
        printf("error, for A=1, B=0, C=0, D=1 result should be 1.\n");
    if (BDD_use(bdd, "1101")[0] != '1')
        printf("error, for A=1, B=1, C=0, D=1 result should be 1.\n");
    if (BDD_use(bdd, "1110")[0] != '1')
        printf("error, for A=1, B=1, C=1, D=0 result should be 1.\n");

    if (BDD_use(bdd, "1111")[0] != '0')
        printf("error, for A=1, B=1, C=1, D=1 result should be 0.\n");
    if (BDD_use(bdd, "0110")[0] != '0')
        printf("error, for A=0, B=1, C=1, D=0 result should be 0.\n");
    if (BDD_use(bdd, "0101")[0] != '0')
        printf("error, for A=0, B=1, C=0, D=1 result should be 0.\n");
    if (BDD_use(bdd, "0010")[0] != '0')
        printf("error, for A=0, B=0, C=1, D=0 result should be 0.\n");
    if (BDD_use(bdd, "1011")[0] != '0')
        printf("error, for A=1, B=0, C=1, D=1 result should be 0.\n");


    return 0;
}
