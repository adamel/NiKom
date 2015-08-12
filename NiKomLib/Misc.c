#include "/Include/NiKomStr.h"
#include "/Include/NiKomLib.h"
#include "NiKomBase.h"

#include "funcs.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <exec/memory.h>
#include <proto/exec.h>

/****** nikom.library/NiKParse ******************************************

    NAME
        NiKParse  -- Parsrar olika typer av information och ger ett nummer som svar
    SYNOPSIS
        nummer = NiKParse( string, subjekt )
        d0                 a0      d0
        int NiKParse( char * , char * )

    FUNCTION

		Parsrar ett m�tesnamn, areanamn, kommandonamn, anv�ndarnamn
		eller nyckelnamn till resp. nummer.

    INPUTS

		Subject kan vara f�ljande:
		k - kommando
		m - m�tesnamn
		n - anv�ndarnamn
		a - areanamn
		y - nyckelnamn

		Str�ng �r en str�ng inneh�llande ett namn p� n�got som
		visas av subjektet.

    RESULT

        Numret p� den typ som �nskades.
        
        Returnerar:
        -1 om namnet p� "string":en inte kunde hittas f�r det aktuella subjektet.
        -2 Om subjektet inte finns.
        

    EXAMPLE

		say NiKParse('ni li','n') ==> 0
		variabel="li m�"
		say NiKParse(variabel,'k') ==> 101

    NOTES

    BUGS

    SEE ALSO
        

******************************************************************************
*
*/

int __saveds __asm LIBNiKParse(register __a0 char *string, register __d0 char subject, register __a6 struct NiKomBase *NiKomBase)
{
	int nummer;

	switch(subject)
	{
		case 'k' : case 'K' :
			nummer = parsekom(string, NiKomBase);
			break;
		case 'm' : case 'M' :
			nummer = parsemot(string, NiKomBase);
			break;
		case 'n' : case 'N' :
			nummer = parsenamn(string, NiKomBase);
			break;
		case 'a' : case 'A' :
			nummer = parsearea(string, NiKomBase);
			break;
		case 'y' : case 'Y' :
			nummer = parsenyckel(string, NiKomBase);
			break;
		default :
			nummer = -2;
			break;
	}	
	
	return nummer;
}

/****** nikom.library/SysInfo ******************************************

    NAME
        SysInfo  -- 
    SYNOPSIS
        nummer = SysInfo( subject )
        d0                a0
		int SysInfo( char * )

    FUNCTION

		Parsrar ett m�tesnamn, areanamn, kommandonamn, anv�ndarnamn
		eller nyckelnamn till resp. nummer.

    INPUTS

		Subject kan vara f�ljande:
		
		a - h�gsta anv�ndarnummer
		n - antal nycklar
		h - h�gsta textnummer
		k - antal kommandon
		l - l�gsta textnummer
		m - h�gsta m�tesnummer
		o - h�gsta areanummer
		bx - Returnerar en str�ng med bps och antalet som ringt med denna
			 bps f�r nummer bps connect nummer x separerat med ett mellanslag.
		c - 
		t - Totalt antal olika bps connects som BBSen haft. (anv�nds tillsammans
			med bx f�r att f� fram olika connect hastigheter till BBSen, och hur
			m�nga av varje.)

    RESULT

        V�rdet p� den associerat till subjektet eller -1 om subjektet inte finns.

    EXAMPLE

		printf("%d\n",SysInfo("h")); ==> 6648
		puts(SysInfo("b5")) ==> 14400 144

    NOTES

    BUGS

    SEE ALSO
        

******************************************************************************
*
*/

int __saveds __asm LIBSysInfo(register __a0 char *subject, register __a6 struct NiKomBase *NiKomBase)
{
	int varde;

	switch(subject[0])
	{
		case 'a' : case 'A' :
			return (int)((struct ShortUser *)NiKomBase->Servermem->user_list.mlh_TailPred)->nummer;
			break;

		case 'n' : case 'N' :
			return (int)NiKomBase->Servermem->info.nycklar;
			break;

		case 'h' : case 'H' :
			return (int)NiKomBase->Servermem->info.hightext;
			break;
		
		case 'k' : case 'K' :
			return (int)NiKomBase->Servermem->info.kommandon;
			break;
		
		case 'l' : case 'L' :
			return (int)NiKomBase->Servermem->info.lowtext;
			break;
		
		case 'm' : case 'M' :
			return (int)NiKomBase->Servermem->info.moten;
			break;
		
		case 'o' : case 'O' :
			return (int)NiKomBase->Servermem->info.areor;
			break;
		
		case 'b' : case 'B' :
			varde = atoi(&subject[1]);
			if(varde > -1 && varde <= countbps(NiKomBase))
				return (int)NiKomBase->Servermem->info.bps[varde];
			else
				return -2;
			break;

		case 't' : case 'T' :
			return countbps(NiKomBase);
			break;

		otherwise :
			return -1;
	}
}

int countbps(struct NiKomBase *NiKomBase)
{
	int antbps = 0;
	
	while(NiKomBase->Servermem->info.bps[antbps] > 0)
		antbps++;

	return antbps;
}

/****** nikom.library/CommandInfo ******************************************

    NAME
        CommandInfo  -- L�ser in information om en anv�ndare.
    SYNOPSIS
        nummer = CommandInfo(kommandonummer, subject )
        d0                	  d0			,	a0

        int CommandInfo( ULONG , char * )

    FUNCTION

		Parsrar ett m�tesnamn, areanamn, kommandonamn, anv�ndarnamn
		eller nyckelnamn till resp. nummer.

    INPUTS

		Subject kan vara f�ljande:
		a - argument (0=inget, 1=numeriskt, 2=annat)
		d - Antal dagar man ska ha varit registrerad f�r att anv�nda komamndot.
		l - Antal inloggningar man m�ste ha gjort f�r att anv�nda kommandot.
		n - Namnet
		o - Antal ord, 1 eller 2.
		s - Status man m�ste ha f�r att anv�nda kommandot.
		x - L�senord som beh�vs f�r att anv�nda komamndot.

    RESULT

        V�rdet p� den associerat till subjektet och det angivna
        kommandonumret.
        -1 om kommandot inte finns.

    EXAMPLE

		printf("%d\n",CommandInfo("h")); ==> 6648
		puts(CommandInfo("b5")) ==> 14400 144

    NOTES

    BUGS

    SEE ALSO

CommandInfo(kommandonr,subject)

F�ljande subject finns:
a - argument (0=inget, 1=numeriskt, 2=annat)
d - Antal dagar man ska ha varit registrerad f�r att anv�nda komamndot.
l - Antal inloggningar man m�ste ha gjort f�r att anv�nda kommandot.
n - Namnet
o - Antal ord, 1 eller 2.
s - Status man m�ste ha f�r att anv�nda kommandot.
x - L�senord som beh�vs f�r att anv�nda komamndot.

Felv�rde:
1 - Inte tillr�ckligt med argument.

Returv�rden:
-1 - Finns inget kommando med det numret.
Annars den beg�rda informationen.

Ex:
say CommandInfo(102,'n') --> LISTA ANV�NDARE
        

******************************************************************************
*
*/


/****** nikom.library/SendNodeMess ******************************************

    NAME
        SendNodeMess  -- L�ser in information om en anv�ndare.
    SYNOPSIS
        nummer = SendNodeMess( subject )
        d0                a0
        int NiKParse( char * , char * )

    FUNCTION

		Parsrar ett m�tesnamn, areanamn, kommandonamn, anv�ndarnamn
		eller nyckelnamn till resp. nummer.

    INPUTS

		Subject kan vara f�ljande:
		
		a - h�gsta anv�ndarnummer
		n - antal nycklar
		h - h�gsta textnummer
		k - antal kommandon
		l - l�gsta textnummer
		m - h�gsta m�tesnummer
		o - h�gsta areanummer
		bx - Returnerar en str�ng med bps och antalet som ringt med denna
			 bps f�r nummer bps connect nummer x separerat med ett mellanslag.
		t - Totalt antal olika bps connects som BBSen haft. (anv�nds tillsammans
			med bx f�r att f� fram olika connect hastigheter till BBSen, och hur
			m�nga av varje.)

    RESULT

        V�rdet p� den associerat till subjektet.

    EXAMPLE

		printf("%d\n",SendNodeMess("h")); ==> 6648
		puts(SendNodeMess("b5")) ==> 14400 144

    NOTES

    BUGS

    SEE ALSO
        

******************************************************************************
*
*/

/* Funktioner som anv�nds i olika delar av libraryts rutiner... */

int parsekom(char *skri, struct NiKomBase *NiKomBase)
{
	int nummer=0,argtyp;
	char *arg2,*ord2;
	struct Kommando *kompek,*forst=NULL;
	if(skri[0]==0) return(-3);
	if(isdigit(skri[0])) {
		/* argument=skri; */
		return(212);
	}
	arg2=hittaefter(skri);
	if(isdigit(arg2[0])) argtyp=KOMARGNUM;
	else if(!arg2[0]) argtyp=KOMARGINGET;
	else argtyp=KOMARGCHAR;
	for(kompek=(struct Kommando *)NiKomBase->Servermem->kom_list.mlh_Head;kompek->kom_node.mln_Succ;kompek=(struct Kommando *)kompek->kom_node.mln_Succ) {
		if(matchar(skri,kompek->namn)) {
			ord2=hittaefter(kompek->namn);
			if((kompek->antal_ord==2 && matchar(arg2,ord2) && arg2[0]) || kompek->antal_ord==1) {
				if(kompek->antal_ord==1) {
					if(kompek->argument==KOMARGNUM && argtyp==KOMARGCHAR) continue;
					if(kompek->argument==KOMARGINGET && argtyp!=KOMARGINGET) continue;
				}
				if(forst==NULL) {
					forst=kompek;
					nummer=kompek->nummer;
				}
				else if(forst==(struct Kommando *)1L) {
					/* puttekn(kompek->namn,-1);
					puttekn("\r",-1); */
				} else {
					/* puttekn("\r\n\nFLERTYDIGT KOMMANDO\r\n\n",-1);
					puttekn(forst->namn,-1);
					puttekn("\r",-1);
					puttekn(kompek->namn,-1);
					puttekn("\r",-1); */
					forst=(struct Kommando *)1L;
				}
			}
		}
	}
	/* if(forst!=NULL && forst!=(struct Kommando *)1L) {
		argument=hittaefter(skri);
		if(forst->antal_ord==2) argument=hittaefter(argument);
	} */
	if(forst==NULL) return(-1);
	else if(forst==(struct Kommando *)1L) return(-2);
	/* else if(forst->status > NiKomBase->Servermem->inne[nodnr].status ||
		forst->minlogg > NiKomBase->Servermem->inne[nodnr].loggin ||
		forst->mindays*86400 > time(&tid)-NiKomBase->Servermem->inne[nodnr].forst_in) return(-4);
	if(forst->losen[0]) {
		puttekn("\r\n\nL�sen: ",-1);
		getstring(EJEKO,20,NULL);
		if(strcmp(forst->losen,inmat)) return(-5);
	} */
	return(nummer);
}

int parsemot(char *skri, struct NiKomBase *NiKomBase)
{
	struct Mote *motpek=(struct Mote *)NiKomBase->Servermem->mot_list.mlh_Head;
	int going2=TRUE,found=-1;
	char *faci,*skri2;
	if(skri[0]==0 || skri[0]==' ') return(-3);
	for(;motpek->mot_node.mln_Succ;motpek=(struct Mote *)motpek->mot_node.mln_Succ) {
		faci=motpek->namn;
		skri2=skri;
		going2=TRUE;
		if(matchar(skri2,faci)) {
			while(going2) {
				skri2=hittaefter(skri2);
				faci=hittaefter(faci);
				if(skri2[0]==0) return((int)motpek->nummer);
				else if(faci[0]==0) going2=FALSE;
				else if(!matchar(skri2,faci)) {
					faci=hittaefter(faci);
					if(faci[0]==0 || !matchar(skri2,faci)) going2=FALSE;
				}
			}
		}
	}
	return(found);
}

int parsearea(char *skri, struct NiKomBase *NiKomBase)
{
	int going=TRUE,going2=TRUE,count=0,found=-1;
	char *faci,*skri2;
	if(skri[0]==0 || skri[0]==' ') {
		found=-3;
		going=FALSE;
	}
	while(going) {
		if(count==NiKomBase->Servermem->info.areor) going=FALSE;
		faci=NiKomBase->Servermem->areor[count].namn;
		skri2=skri;
		going2=TRUE;
		if(matchar(skri2,faci)) {
			while(going2) {
				skri2=hittaefter(skri2);
				faci=hittaefter(faci);
				if(skri2[0]==0) {
					found=count;
					going2=going=FALSE;
				}
				else if(faci[0]==0) going2=FALSE;
				else if(!matchar(skri2,faci)) {
					faci=hittaefter(faci);
					if(faci[0]==0 || !matchar(skri2,faci)) going2=FALSE;
				}
			}
		}
		count++;
	}
	return(found);
}

int parsenyckel(char *skri, struct NiKomBase *NiKomBase)
{
	int going=TRUE,going2=TRUE,count=0,found=-1;
	char *faci,*skri2;
	if(skri[0]==0 || skri[0]==' ') {
		found=-3;
		going=FALSE;
	}
	while(going) {
		if(count==NiKomBase->Servermem->info.nycklar) going=FALSE;
		faci=NiKomBase->Servermem->Nyckelnamn[count];
		skri2=skri;
		going2=TRUE;
		if(matchar(skri2,faci)) {
			while(going2) {
				skri2=hittaefter(skri2);
				faci=hittaefter(faci);
				if(skri2[0]==0) {
					found=count;
					going2=going=FALSE;
				}
				else if(faci[0]==0) going2=FALSE;
				else if(!matchar(skri2,faci)) {
					faci=hittaefter(faci);
					if(faci[0]==0 || !matchar(skri2,faci)) going2=FALSE;
				}
			}
		}
		count++;
	}
	return(found);
}

void MakeUserFilePath(char* string, int userId, char *fileName) {
  sprintf(string, "NiKom:Users/%d/%d/%s", userId/100, userId, fileName);
}
