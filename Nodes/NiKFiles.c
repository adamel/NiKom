#include <exec/types.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <error.h>
#include "NiKomstr.h"
#include "NiKomFuncs.h"
#include "DiskUtils.h"
#include "Terminal.h"

#define ERROR	10
#define OK		0
#define EKO		1
#define EJEKO	0
#define KOM		1
#define EJKOM	0

extern struct System *Servermem;
extern int nodnr,inloggad,area2,rad;
extern char outbuffer[],inmat[],*argument;
extern struct Inloggning Statstr;
extern struct MinList edit_list;

int writefiles(int area)
{
	struct Node *nod;
	BPTR fh;
	char datafil[110];
	int index;

	ObtainSemaphore(&Servermem->semaphores[NIKSEM_FILEAREAS]);

	sprintf(datafil,"nikom:datocfg/areor/%d.dat",area);

	if(!(fh=Open(datafil,MODE_NEWFILE)))
	{
		ReleaseSemaphore(&Servermem->semaphores[NIKSEM_FILEAREAS]);
		return 1;
	}

	index=0;
	nod = ((struct List *)&Servermem->areor[area].ar_list)->lh_Head;
	while(nod->ln_Succ)
	{
		if(Write(fh,nod,sizeof(struct DiskFil)) != sizeof(struct DiskFil)) {
			Close(fh);
			ReleaseSemaphore(&Servermem->semaphores[NIKSEM_FILEAREAS]);
			return 3;
		}
		((struct Fil *)nod)->index = index++;
		nod = nod->ln_Succ;
	}
	Close(fh);

	ReleaseSemaphore(&Servermem->semaphores[NIKSEM_FILEAREAS]);

	return 0;
}

int updatefile(int area,struct Fil *fil)
{
	BPTR fh;
	char datafil[110];

	ObtainSemaphore(&Servermem->semaphores[NIKSEM_FILEAREAS]);

	sprintf(datafil,"nikom:datocfg/areor/%d.dat",area);

	if(!(fh=Open(datafil,MODE_OLDFILE)))
	{
		ReleaseSemaphore(&Servermem->semaphores[NIKSEM_FILEAREAS]);
		return 1;
	}

	if(Seek(fh,fil->index * sizeof(struct DiskFil),OFFSET_BEGINNING)==-1)
	{
		ReleaseSemaphore(&Servermem->semaphores[NIKSEM_FILEAREAS]);
		Close(fh);
		return 2;
	}

	if(Write(fh,fil,sizeof(struct DiskFil)) != sizeof(struct DiskFil))
	{
		ReleaseSemaphore(&Servermem->semaphores[NIKSEM_FILEAREAS]);
		Close(fh);
		return 3;
	}

	Seek(fh,0,OFFSET_END);
	Close(fh);
	ReleaseSemaphore(&Servermem->semaphores[NIKSEM_FILEAREAS]);
	return 0;
}

int skaparea(void) {
	BPTR fh;
	long tid;
	char tempdir[101];
	int going=TRUE,x,old=Servermem->info.areor,ret=0,y;
	for(x=0;x<MAXAREA;x++) {
		if(!Servermem->areor[x].namn[0]) break;
	}
	if(x==MAXAREA) {
		puttekn("\r\n\nFinns ej plats f�r fler areor!!\r\n",-1);
		return(0);
	}
	if(x==Servermem->info.areor) Servermem->info.areor++;
	memset(&Servermem->areor[x],0,sizeof(struct Area));
	if(argument[0]==0) {
		puttekn("\r\n\nNamn p� arean: ",-1);
		if(getstring(EKO,40,NULL)) { Servermem->info.areor=old; return(1); }
		if(parsearea(inmat)!=-1) { Servermem->info.areor=old; return(0); }
		strcpy(Servermem->areor[x].namn,inmat);
	} else strcpy(Servermem->areor[x].namn,argument);
	time(&tid);
	Servermem->areor[x].skapad_tid=tid;
	Servermem->areor[x].skapad_av=inloggad;
	Servermem->areor[x].filer=0;
	Servermem->areor[x].status=0;
	NewList((struct List *)&Servermem->areor[x].ar_list);
	while(going) {
		puttekn("\r\nTill vilket m�te ska arean h�ra? (<RETURN> f�r inget) ",-1);
		if(getstring(EKO,40,NULL)) { Servermem->info.areor=old; Servermem->areor[x].namn[0]=0; return(1); }
		if((ret=parsemot(inmat))>=0) {
			Servermem->areor[x].mote=ret;
			sprintf(outbuffer,"\r\nKopplar arean till m�tet %s",getmotnamn(ret));
			puttekn(outbuffer,-1);
			going=FALSE;
		} else if(ret==-3) {
			Servermem->areor[x].mote=-1;
			going=FALSE;
		} else puttekn("\r\nFinns inget s�dant m�te!",-1);
	}
	puttekn("\n\rSka uploads till arean till�tas? ",-1);
	if(!jaellernej('j','n',1)) {
		Servermem->areor[x].flaggor |= AREA_NOUPLOAD;
		puttekn("Nej",-1);
	} else puttekn("Ja",-1);
	puttekn("\n\rSka downloads fr�n arean till�tas? ",-1);
	if(!jaellernej('j','n',1)) {
		Servermem->areor[x].flaggor |= AREA_NODOWNLOAD;
		puttekn("Nej",-1);
	} else puttekn("Ja",-1);
	puttekn("\r\nVilka grupper ska ha tillg�ng till arean? (? f�r lista)\r\n",-1);
	Servermem->areor[x].grupper=0L;
	if(editgrupp((char *)&Servermem->areor[x].grupper)) { Servermem->info.areor=old; Servermem->areor[x].namn[0]=0; return(1); }
	puttekn("\r\nTill vilka directoryn ska arean h�ra?",-1);
	for(y=0;y<16;y++) Servermem->areor[x].dir[y][0]=0;
	for(y=0;y<16;y++)
	{
		while(!Servermem->areor[x].dir[y][0])
		{
			sprintf(outbuffer,"\r\nDirectory %d: ",y+1);
			puttekn(outbuffer,-1);
			inmat[0] = 0;
			if(getstring(EKO,39,NULL)) { Servermem->info.areor=old; return(1); }
			if(!inmat[0]) break;
			ret=strlen(inmat);
			if(inmat[ret-1]!='/' && inmat[ret-1]!=':') {
				inmat[ret]='/';
				inmat[ret+1]=0;
			}
			strcpy(Servermem->areor[x].dir[y],inmat);
			strcpy(tempdir, inmat);
			tempdir[strlen(tempdir)-1] = 0;
			if(mkdir(tempdir))
			{
				Servermem->areor[x].dir[y][0] = 0;
				puttekn("\r\nKunde inte skapa biblioteket!", -1);
			}
		}
		if(!inmat[0]) break;
		sprintf(tempdir,"%slongdesc",Servermem->areor[x].dir[y]);
		mkdir(tempdir);
		puttekn("\r\nNycklar till directoryt.\r\n",-1);
		editkey(Servermem->areor[x].nycklar[y]);
	}
	NiKForbid();
	if(!(fh=Open("NiKom:DatoCfg/Areor.dat",MODE_OLDFILE))) {
		puttekn("\r\n\nKunde inte �ppna Areor.dat\r\n",-1);
		Servermem->info.areor=old;
		Servermem->areor[x].namn[0]=0;
		NiKPermit();
		return(0);
	}
	if(Write(fh,(void *)Servermem->areor,sizeof(struct Area)*Servermem->info.areor)==-1) {
		puttekn("\r\n\nFel vid skrivandet av M�ten.dat\r\n",-1);
		Servermem->info.areor=old;
		Servermem->areor[x].namn[0]=0;
	}
	Close(fh);
	NiKPermit();
	return(0);
}

int parsearea(skri)
char *skri;
{
	int going=TRUE,going2=TRUE,count=0,found=-1;
	char *faci,*skri2;
	if(skri[0]==0 || skri[0]==' ') {
		found=-3;
		going=FALSE;
	}
	while(going) {
		if(count==Servermem->info.areor) going=FALSE;
		faci=Servermem->areor[count].namn;
		skri2=skri;
		going2=TRUE;
		if(!arearatt(count, inloggad, &Servermem->inne[nodnr])) going2=FALSE;
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

void listarea(void)
{
	int x;
	puttekn("\r\n\n",-1);
	for(x=0;x<Servermem->info.areor;x++) {
		if(!Servermem->areor[x].namn[0] || !arearatt(x, inloggad, &Servermem->inne[nodnr])) continue;
		sprintf(outbuffer,"%s",Servermem->areor[x].namn);
		puttekn(outbuffer,-1);
		if(Servermem->areor[x].flaggor & AREA_NOUPLOAD) puttekn(" (Ingen upload)",-1);
		if(Servermem->areor[x].flaggor & AREA_NODOWNLOAD) puttekn(" (Ingen download)",-1);
		puttekn("\n\r",-1);
	}
	eka('\n');
}

int parsenyckel(skri)
char *skri;
{
	int going=TRUE,going2=TRUE,count=0,found=-1;
	char *faci,*skri2;
	if(skri[0]==0 || skri[0]==' ') {
		found=-3;
		going=FALSE;
	}
	while(going) {
		if(count==Servermem->info.nycklar) going=FALSE;
		faci=Servermem->Nyckelnamn[count];
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

void listnyckel(void) {
	int x;
	puttekn("\r\n\n",-1);
	for(x=0;x<Servermem->info.nycklar;x++) {
		if(puttekn(Servermem->Nyckelnamn[x],-1)) return;
		eka('\r');
	}
	puttekn("\r\n",-1);
}

void listfiler(void) {
	struct Fil *sokpek;
	struct tm *ts;
	int nyckel=-1,notvalid=FALSE;
	if(area2==-1) {
		puttekn("\r\n\nDu befinner dig inte i n�gon area!\r\n",-1);
		return;
	}
	if(argument[0]=='-') {
		notvalid=TRUE;
		argument=hittaefter(argument);
	}
	if(argument[0])
		if((nyckel=parsenyckel(argument))==-1) {
			puttekn("\r\n\nFinns ingen s�dan nyckel!\r\n",-1);
			return;
		}
	sprintf(outbuffer,"\r\n\nFiler i arean %s\n",Servermem->areor[area2].namn);
	puttekn(outbuffer,-1);
	for(sokpek=(struct Fil *)Servermem->areor[area2].ar_list.mlh_TailPred;sokpek->f_node.mln_Pred;sokpek=(struct Fil *)sokpek->f_node.mln_Pred) {
		if(notvalid && !(sokpek->flaggor & FILE_NOTVALID)) continue;
		if((sokpek->flaggor & FILE_NOTVALID) && Servermem->inne[nodnr].status < Servermem->cfg.st.filer && sokpek->uppladdare != inloggad) continue;
		if(nyckel!=-1 && !BAMTEST(sokpek->nycklar,nyckel)) continue;
		ts=localtime(&sokpek->tid);
		if(puttekn("\r\n",-1)) return;
		sprintf(outbuffer,"%-24s %c%c %7d %02d%02d%02d %-28s %2d\r\n",
		              sokpek->namn,
		              sokpek->flaggor & FILE_NOTVALID ? 'V' : ' ',
		              sokpek->flaggor & FILE_FREEDL ? 'F' : ' ',
						  sokpek->size,
						  ts->tm_year % 100,
						  ts->tm_mon+1,
						  ts->tm_mday,
						  getusername(sokpek->uppladdare),
						  sokpek->downloads);
		if(puttekn(outbuffer,-1)) return;
		if(sokpek->flaggor & FILE_LONGDESC) sprintf(outbuffer,"**> %s\n",sokpek->beskr);
		else sprintf(outbuffer,"    %s\n",sokpek->beskr);
		if(puttekn(outbuffer,-1)) return;
	}
}

void bytarea(void) {
	int area;
	if((area=parsearea(argument))==-3) {
		if(area2==-1) puttekn("\n\n\rDu befinner dig inte i n�gon area.\n\r",-1);
		else {
			sprintf(outbuffer,"\n\n\rDu befinner dig i arean %s\n\r",Servermem->areor[area2].namn);
			puttekn(outbuffer,-1);
		}
		return;
	}
	if(area==-1) {
		puttekn("\r\n\nFinns ingen s�dan area!\r\n",-1);
		return;
	}
/*	if(!arearatt(area, inloggad, &Servermem->inne[nodnr])) {
		puttekn("\r\n\nFinns ingen s�dan area!\r\n",-1);
		return;
	} */
	area2=area;
	sprintf(outbuffer,"\r\n\nDu befinner dig nu i arean %s.\r\n",Servermem->areor[area].namn);
	puttekn(outbuffer,-1);
}

void filinfo(void) {
	if(area2==-1) {
		puttekn("\r\n\nDu befinner dig inte i n�gon area.\r\n",-1);
	} else {
		sprintf(outbuffer,"\r\n\nDu befinner dig i arean %s.\r\n\n",Servermem->areor[area2].namn);
		puttekn(outbuffer,-1);
	}
	/*
	if(Servermem->inne[nodnr].protokoll==ZMODEM) sprintf(outbuffer,"Du har Zmodem inst�llt som protokoll.\r\n");
	else if(Servermem->inne[nodnr].protokoll==XMODEM) sprintf(outbuffer,"Du har Xmodem inst�llt som protokoll.\r\n");
	else sprintf(outbuffer,"Du har inget protokoll inst�llt\r\n");
	puttekn(outbuffer,-1);
	*/
}

void radarea(void) {
	int arnr;
	BPTR fh;
	if(!argument[0]) {
		puttekn("\r\n\nSkriv: Radera Area <areanamn>\r\n",-1);
		return;
	}
	if((arnr=parsearea(argument))==-1) {
		puttekn("\r\n\nFinns ingen s�dan area!\r\n",-1);
		return;
	}
	sprintf(outbuffer,"\n\n\r�r du s�ker p� att du vill radera arean %s? ",Servermem->areor[arnr].namn);
	puttekn(outbuffer,-1);
	if(!jaellernej('j','n',2)) return;
	Servermem->areor[arnr].namn[0]=0;
	NiKForbid();
	if(!(fh=Open("NiKom:DatoCfg/Areor.dat",MODE_OLDFILE))) {
		puttekn("\r\n\nKunde inte �ppna Areor.dat\r\n",-1);
		NiKPermit();
		return;
	}
	if(Seek(fh,arnr*sizeof(struct Area),OFFSET_BEGINNING)==-1) {
		puttekn("\r\nKunde inte s�ka i Areor.dat!\r\n",-1);
		Close(fh);
		NiKPermit();
		return;
	}
	if(Write(fh,(void *)&Servermem->areor[arnr],sizeof(struct Area))==-1) puttekn("\r\n\nFel vid skrivandet av M�ten.dat\r\n",-1);
	Close(fh);
	NiKPermit();
}

int andraarea(void) {
	int arnr,foo,ret,y,dir;
	struct Area tempar;
	char tempdir[101];
	BPTR fh;
	if((arnr=parsearea(argument))==-1) {
		puttekn("\r\n\nFinns ingen area som heter s�!\r\n\n",-1);
		return(0);
	} else if(arnr==-3) {
		puttekn("\r\n\nSkriv: �ndra Area <areanamn>\r\n\n",-1);
		return(0);
	}
	if(inloggad!=Servermem->areor[arnr].skapad_av && Servermem->inne[nodnr].status<Servermem->cfg.st.radarea) {
		puttekn("\r\n\nDu har ingen r�tt att �ndra p� den arean!\r\n\n",-1);
		return(0);
	}
	memcpy(&tempar,&Servermem->areor[arnr],sizeof(struct Area));
	for(;;) {
		sprintf(outbuffer,"\r\n\nNamn: (%s) ",tempar.namn);
		puttekn(outbuffer,-1);
		if(getstring(EKO,40,NULL)) return(1);
		if(!inmat[0]) break;
		if((foo=parsearea(inmat))!=-1 && foo!=arnr) {
			puttekn("\r\nFinns redan en s�dan area!",-1);
			continue;
		}
		strcpy(tempar.namn,inmat);
		break;
	}
	for(;;) {
		if(tempar.mote!=-1) sprintf(outbuffer,"\r\nKopplad till: (%s)\r\n'-' f�r inget m�te\r\n",getmotnamn(tempar.mote));
		else strcpy(outbuffer,"\r\nKopplad till: (<Inget m�te>) ");
		puttekn(outbuffer,-1);
		if(getstring(EKO,40,NULL)) return(1);
		if(!inmat[0]) break;
		if(inmat[0]=='-') {
			tempar.mote=-1;
			break;
		}
		if((foo=parsemot(inmat))==-1) {
			puttekn("\r\nFinns inget s�dant m�te!",-1);
			continue;
		}
		tempar.mote=foo;
		break;
	}
	puttekn("\n\rSka uploads till arean till�tas? ",-1);
	if(!jaellernej('j','n',(tempar.flaggor & AREA_NOUPLOAD) ? 2 : 1)) {
		tempar.flaggor |= AREA_NOUPLOAD;
		puttekn("Nej",-1);
	} else {
		puttekn("Ja",-1);
		tempar.flaggor &= ~AREA_NOUPLOAD;
	}
	puttekn("\n\rSka downloads fr�n arean till�tas? ",-1);
	if(!jaellernej('j','n',(tempar.flaggor & AREA_NODOWNLOAD) ? 2 : 1)) {
		tempar.flaggor |= AREA_NODOWNLOAD;
		puttekn("Nej",-1);
	} else {
		puttekn("Ja",-1);
		tempar.flaggor &= ~AREA_NODOWNLOAD;
	}
	puttekn("\r\nVilka grupper ska ha tillg�ng till arean?\r\n! f�r att se hur det �r nu, ? f�r lista\r\n",-1);
	if(editgrupp((char *)&tempar.grupper)) return(0);
	puttekn("\r\nDirectoryn\r\n",-1);
	for(y=0;y<16;y++) {
		if(tempar.dir[y][0]) sprintf(outbuffer,"%2d: %s\r\n",y,tempar.dir[y]);
		else sprintf(outbuffer,"%2d: <Inget directory>\r\n",y);
		puttekn(outbuffer,-1);
	}
	for(;;) {
		puttekn("\r\n�ndra vilket directory? ",-1);
		if(getstring(EKO,3,NULL)) return(1);
		if(!inmat[0]) break;
		dir=atoi(inmat);
		if(dir<0 || dir>15) {
			puttekn("\r\nFelaktigt nummer!",-1);
			continue;
		}
		puttekn("\r\n�ndra till: ('-' f�r att ta bort) ",-1);
		if(getstring(EKO,40,tempar.dir[dir])) return(1);
		if(!inmat[0]) continue;
		if(inmat[0]=='-') {
			tempar.dir[dir][0]=0;
			memset(tempar.nycklar[dir],0,MAXNYCKLAR/8);
			continue;
		}
		ret=strlen(inmat);
		if(inmat[ret-1]!='/' && inmat[ret-1]!=':') {
			inmat[ret]='/';
			inmat[ret+1]=0;
		}
		strcpy(tempar.dir[dir],inmat);
		strcpy(tempdir, inmat);
		tempdir[strlen(tempdir)-1] = 0;
		mkdir(tempdir);
		sprintf(tempdir,"%slongdesc", tempar.dir[dir]);
		mkdir(tempdir);
		puttekn("\r\nOch nycklarna till det directoryt...\r\n",-1);
		editkey(tempar.nycklar[dir]);
	}
	puttekn("\r\n\n�r allt korrekt? ",-1);
	if(!jaellernej('j','n',1)) return(0);
	memcpy(&Servermem->areor[arnr],&tempar,sizeof(struct Area));
	NiKForbid();
	if(!(fh=Open("NiKom:DatoCfg/Areor.dat",MODE_OLDFILE))) {
		puttekn("\r\n\nKunde inte �ppna Areor.dat\r\n",-1);
		NiKPermit();
		return(0);
	}
	if(Seek(fh,arnr*sizeof(struct Area),OFFSET_BEGINNING)) {
		puttekn("\r\nKunde inte s�ka i Areor.dat!\r\n",-1);
		Close(fh);
		NiKPermit();
		return(0);
	}
	if(Write(fh,(void *)&Servermem->areor[arnr],sizeof(struct Area))==-1) puttekn("\r\n\nFel vid skrivandet av M�ten.dat\r\n",-1);
	Close(fh);
	NiKPermit();
	return(0);
}

int skapafil(void) {
	struct Fil *allokpek;
	struct EditLine *el;
	int area, editret, y, isCorrect;
	long tid;
	BPTR fh;
	__aligned struct FileInfoBlock info;
	char filnamn[41],fullpath[100],nikfilename[109];
	if(!argument[0]) {
		puttekn("\r\n\nNamn p� filen: ",-1);
		if(getstring(EKO,40,NULL)) return(1);
		if(!inmat[0]) return(0);
		argument=inmat;
	}
	strcpy(filnamn,argument);
	if(area2==-1) {
		puttekn("\r\nI vilken area? ",-1);
	} else {
		sprintf(outbuffer,"\r\nI vilken area? (<RETURN> f�r %s)",Servermem->areor[area2].namn);
		puttekn(outbuffer,-1);
	}
	if(getstring(EKO,40,NULL)) return(1);
	if((area=parsearea(inmat))==-1) {
		puttekn("\r\nFinns ingen s�dan area!\r\n",-1);
		return(0);
	} else if(area==-3) {
		if(area2==-1) return(0);
		else area=area2;
	}
	if(!valnamn(filnamn,area,fullpath)) { /* Bara l�nar fullpath... */
		puttekn(fullpath,-1);
		return(0);
	}
	for(y=0;y<16;y++) {
		if(!Servermem->areor[area].dir[y][0]) continue;
		sprintf(fullpath,"%s%s",Servermem->areor[area].dir[y],filnamn);
		if(!access(fullpath,0)) break;
	}
	if(y==16) {
		sprintf(outbuffer,"\r\nFilen %s finns inte\r\n",fullpath);
		puttekn(outbuffer,-1);
		return(0);
	}
	sprintf(fullpath,"%s%s",Servermem->areor[area].dir[y],filnamn);
	if(!(allokpek=(struct Fil *)AllocMem(sizeof(struct Fil),MEMF_PUBLIC | MEMF_CLEAR))) {
		puttekn("\r\n\nKunde inte allokera minne f�r filen!\r\n",-1);
		return(0);
	}
	strcpy(allokpek->namn,filnamn);
	time(&tid);
	allokpek->dir=y;
	allokpek->tid=allokpek->validtime=tid;
	allokpek->uppladdare=inloggad;
	if(dfind(&info,fullpath,0)) {
		puttekn("\r\nNu finns filen helt pl�tsligt inte l�ngre!?\r\n",-1);
		FreeMem(allokpek,sizeof(struct Fil));
		return(0);
	}
	allokpek->size=info.fib_Size;
	puttekn("\r\nVilken statusniv� ska filen ha? ",-1);
	if(getstring(EKO,3,NULL)) { FreeMem(allokpek,sizeof(struct Fil)); return(1); }
	allokpek->status=atoi(inmat);
	if(Servermem->inne[nodnr].status >= Servermem->cfg.st.filer) {
		puttekn("\n\rSka filen valideras? ",-1);
		if(jaellernej('j','n',1)) puttekn("Ja",-1);
		else {
			puttekn("Nej",-1);
			allokpek->flaggor|=FILE_NOTVALID;
		}
		puttekn("\n\rSka filen ha fri download? ",-1);
		if(jaellernej('j','n',2)) {
			puttekn("Ja",-1);
			allokpek->flaggor|=FILE_FREEDL;
		}
		else 	puttekn("Nej",-1);
	}
	puttekn("\r\nVilka s�knycklar ska filen ha? (? f�r att f� en lista)\r\n",-1);
	if(editkey(allokpek->nycklar)) { FreeMem(allokpek,sizeof(struct Fil)); return(1); }
	puttekn("\r\nBeskrivning:\r\n",-1);
	if(getstring(EKO,70,NULL)) { FreeMem(allokpek,sizeof(struct Fil)); return(1); }
	strcpy(allokpek->beskr,inmat);
	AddTail((struct List *)&Servermem->areor[area].ar_list,(struct Node *)allokpek);
	if(writefiles(area)) {
		puttekn("\r\n\nKunde inte skriva till datafilen\r\n",-1);
		return(0);
	}
	Servermem->inne[nodnr].upload++;
	Statstr.ul++;

        if(GetYesOrNo("\r\n\nVill du skriva en l�ngre beskrivning?",
                      'j', 'n', "Ja\r\n\n", "Nej\r\n\n", TRUE, &isCorrect)) {
          return 1;
        }
        if(!isCorrect) {
          return 0;
        }
        SendString("\r\n\nOk, g�r in i editorn.\r\n");

	if((editret=edittext(NULL))==1) return(1);
	else if(editret==2) return(0);
	sprintf(nikfilename,"%slongdesc/%s.long",Servermem->areor[area].dir[allokpek->dir],allokpek->namn);
	if(!(fh=Open(nikfilename,MODE_NEWFILE))) {
		puttekn("\r\n\nKunde inte �ppna longdesc-filen\r\n",-1);
		freeeditlist();
		return(0);
	}
	for(el=(struct EditLine *)edit_list.mlh_Head;el->line_node.mln_Succ;el=(struct EditLine *)el->line_node.mln_Succ) {
		if(FPuts(fh,el->text)==-1) {
			freeeditlist();
			Close(fh);
			return(0);
		}
		FPutC(fh,'\n');
	}
	freeeditlist();
	Close(fh);
	puttekn("\r\n",-1);
	allokpek->flaggor|=FILE_LONGDESC;
	return(0);
}

void radfil(void) {
	struct Fil *letpek;
	char filnamn[100],x;
	struct User tempuser;
        int isCorrect;

	if(area2==-1) {
		puttekn("\r\n\nDu befinner dig inte i n�gon area!\r\n",-1);
		return;
	}
	if(!argument[0]) {
		puttekn("\r\n\nSkriv: Radera Fil <filnamn>\r\n",-1);
		return;
	}
	if(!(letpek=parsefil(argument,area2))) {
		puttekn("\r\n\nFinns ingen s�dan fil!\r\n",-1);
		return;
	}
	if(letpek->uppladdare!=inloggad && Servermem->inne[nodnr].status<Servermem->cfg.st.filer) {
		puttekn("\r\n\nDu kan bara radera filer som du sj�lv har laddat upp!\r\n",-1);
		return;
	}

        SendString("\r\n\nRadera filen %s?");
        if(GetYesOrNo(NULL, 'j', 'n', "Ja\r\n", "Nej\r\n", FALSE, &isCorrect)) {
          return;
        }
        if(!isCorrect) {
          return;
        }

	Remove((struct Node *)letpek);

	if(writefiles(area2)) {
		puttekn("\r\nKunde inte uppdatera datafilen.\r\n",-1);
	}

	sprintf(filnamn,"%s%s",Servermem->areor[area2].dir[letpek->dir],letpek->namn);
	if(remove(filnamn)) puttekn("\r\nKunde inte radera filen fysiskt!",-1);
	sprintf(filnamn,"%slongdesc/%s.long",Servermem->areor[area2].dir[letpek->dir],letpek->namn);
	if((letpek->flaggor & FILE_LONGDESC) && remove(filnamn)) puttekn("\r\nKunde inte radera l�nga beskrivningen!",-1);
	if(Servermem->inne[nodnr].status>=Servermem->cfg.st.filer && userexists(letpek->uppladdare)) {
          SendString("\r\nSka %s diskrediteras?", getusername(letpek->uppladdare));
          if(GetYesOrNo(NULL, 'j', 'n', "Ja\r\n", "Nej\r\n", FALSE, &isCorrect)) {
            return;
          }
          if(isCorrect) {
            for(x=0;x<MAXNOD;x++) if(letpek->uppladdare==Servermem->inloggad[x]) break;
            if(x<MAXNOD) {
              Servermem->inne[x].upload--;
            } else {
              if(readuser(letpek->uppladdare,&tempuser)) return;
              tempuser.upload--;
              if(writeuser(letpek->uppladdare,&tempuser)) return;
            }
          }
	} else {
          Servermem->inne[nodnr].upload--;
        }
	FreeMem(letpek,sizeof(struct Fil));
}

int andrafil(void) {
	struct Fil *filpek;
	long tmpflaggor,tmpvalidtime;
	char nikfilename[100],newname[40],oldfullname[100],
		tmpnycklar[MAXNYCKLAR/8],tmpbeskr[71], errbuff[100];
	int x, tmpstatus, tmpuppl, tmpdls, isCorrect;
	UBYTE tn;
	if(area2==-1) {
		puttekn("\r\n\nDu befinner dig inte i n�gon area!\r\n",-1);
		return(0);
	}
	if(!argument[0]) {
		puttekn("\r\n\nSkriv: �ndra Fil <filnamn>\r\n",-1);
		return(0);
	}
	if(!(filpek=parsefil(argument,area2))) {
		puttekn("\r\n\nFinns ingen s�dan fil!\r\n",-1);
		return(0);
	}
	if(filpek->uppladdare!=inloggad && Servermem->inne[nodnr].status<Servermem->cfg.st.filer) {
		puttekn("\r\nDu har ingen r�tt att �ndra p� den filen!\r\n",-1);
		return(0);
	}
	errbuff[0] = 0;
	do {
		if(errbuff[0]) puttekn(errbuff,-1);
		sprintf(outbuffer,"\r\n\nNamn (%s) : ",filpek->namn);
		puttekn(outbuffer,-1);
		if(getstring(EKO,40,NULL)) return(1);
		if(inmat[0] == 0) break;
	} while(!valnamn(inmat,area2,errbuff));
	if(inmat[0]) strcpy(newname,inmat);
	else newname[0]=0;
	memcpy(tmpnycklar,filpek->nycklar,MAXNYCKLAR/8);
	puttekn("\r\nF�ljande nycklar �r satta:\r\n",-1);
	for(x=0;x<Servermem->info.nycklar;x++) if(BAMTEST(tmpnycklar,x)) {
		puttekn(Servermem->Nyckelnamn[x],-1);
		eka('\r');
	}
	if(editkey(tmpnycklar)) return(1);
	sprintf(outbuffer,"\r\nBeskrivning:\r\n(%s)\r\n",filpek->beskr);
	puttekn(outbuffer,-1);
	if(getstring(EKO,70,NULL)) return(1);
	if(inmat[0]) strcpy(tmpbeskr,inmat);
	else strcpy(tmpbeskr,filpek->beskr);
	sprintf(outbuffer,"\r\nStatus f�r att ladda ner filen: (%d) ",filpek->status);
	puttekn(outbuffer,-1);
	if(getstring(EKO,3,NULL)) return(1);
	if(inmat[0]) tmpstatus=atoi(inmat);
	else tmpstatus=filpek->status;
	if(Servermem->inne[nodnr].status>=Servermem->cfg.st.filer) {
		sprintf(outbuffer,"\r\nAntal downloads: (%d) ",filpek->downloads);
		puttekn(outbuffer,-1);
		if(getstring(EKO,7,NULL)) return(1);
		if(inmat[0]) tmpdls=atoi(inmat);
		else tmpdls=filpek->downloads;
		sprintf(outbuffer,"\r\nUppladdare: (%s) ",getusername(filpek->uppladdare));
		puttekn(outbuffer,-1);
		if(getstring(EKO,40,NULL)) return(1);
		if(inmat[0]) {
			if((tmpuppl=parsenamn(inmat))==-1) {
				puttekn("\r\nFinns ingen s�dan anv�ndare!",-1);
				tmpuppl=filpek->uppladdare;
			}
		}
		else tmpuppl=filpek->uppladdare;
		tmpflaggor=filpek->flaggor;
		tmpvalidtime=filpek->validtime;
		puttekn("\n\rSka filen vara validerad? ",-1);
		if(jaellernej('j','n',(tmpflaggor & FILE_NOTVALID) ? 2 : 1)) {
			puttekn("Ja",-1);
			if(tmpflaggor & FILE_NOTVALID) tmpvalidtime=time(NULL);
			tmpflaggor &= ~FILE_NOTVALID;
		} else {
			puttekn("Nej",-1);
			tmpflaggor|=FILE_NOTVALID;
		}
		puttekn("\n\rSka filen ha fri download? ",-1);
		if(jaellernej('j','n',(tmpflaggor & FILE_FREEDL) ? 1 : 2)) {
			puttekn("Ja",-1);
			tmpflaggor|=FILE_FREEDL;
		} else {
			puttekn("Nej",-1);
			tmpflaggor &= ~FILE_FREEDL;
		}
	} else {
		tmpdls=filpek->downloads;
		tmpuppl=filpek->uppladdare;
		tmpflaggor=filpek->flaggor;
		tmpvalidtime=filpek->validtime;
	}

        if(GetYesOrNo("\r\n\n�r allt korrekt?", 'j', 'n', "Ja\r\n\n", "Nej\r\n\n",
                      TRUE, &isCorrect)) {
          return 1;
        }
        if(!isCorrect) {
          return 0;
        }

        strcpy(filpek->beskr,tmpbeskr);
        memcpy(filpek->nycklar,tmpnycklar,MAXNYCKLAR/8);
        filpek->status=tmpstatus;
        filpek->downloads=tmpdls;
        filpek->uppladdare=tmpuppl;
        filpek->flaggor=tmpflaggor;
        filpek->validtime=tmpvalidtime;
        if(newname[0]) {
          sprintf(oldfullname,"%s%s",Servermem->areor[area2].dir[filpek->dir],filpek->namn);
          sprintf(nikfilename,"%s%s",Servermem->areor[area2].dir[filpek->dir],newname);
          rename(oldfullname,nikfilename);
          strcpy(filpek->namn,newname);
        }
        if(updatefile(area2,filpek)) {
          puttekn("\r\n\nKunde inte skriva filen!\r\n",-1);
        }
}

int lagrafil(void) { return(0); }

int flyttafil(void) { return(0); }

int sokfil(void) {
	char sokstring[50],soknyckel[MAXNYCKLAR/8], token[150],upper[80],
		found=FALSE, foundfiles, areatmp, i;
	long uppl,storlek=0,storre=TRUE,going=TRUE,foo,x,pat,fil,keys=FALSE,len;
        int globalSearch;
	struct Fil *filpek;
	struct tm *ts;
	if(area2==-1) {
		puttekn("\r\n\nDu befinner dig inte i n�gon area!\r\n",-1);
		return(0);
	}
	memset(soknyckel,0,MAXNYCKLAR/8);
	if(!argument[0]) {
		puttekn("\r\n\nS�kord: ",-1);
		if(getstring(EKO,49,NULL)) return(1);
		strcpy(sokstring,inmat);
	} else {
		strncpy(sokstring,argument,49);
		sokstring[49] = 0;
		puttekn("\r\n",-1);
	}
	pat=ParsePatternNoCase(sokstring,token,150);
	if(pat) {
		puttekn("\n\rS�ka p� filnamn eller beskrivning? ",-1);
		fil=jaellernej('f','b',1);
		if(fil) puttekn("Fil",-1);
		else puttekn("Beskrivning",-1);
	}

        if(GetYesOrNo("\r\nVill du s�ka globalt?", 'j', 'n', "Ja\r\n", "Nej\r\n",
                      FALSE, &globalSearch)) {
          return 1;
        }

	puttekn("Nycklar att s�ka p�: (? f�r att f� en lista)\r\n",-1);
	if(editkey(soknyckel)) return(1);
	for(x=0;x<MAXNYCKLAR/8;x++) if(soknyckel[x]) keys=TRUE;
	going=TRUE;
	while(going) {
		puttekn("\r\nUppladdare: ",-1);
		if(getstring(EKO,40,NULL)) return(1);
		if(!inmat[0]) { uppl=-1; break; }
		if((uppl=parsenamn(inmat))==-1) {
			puttekn("\r\nFinns ingen s�dan anv�ndare!",-1);
			continue;
		}
		going=FALSE;
	}
	going=TRUE;
	while(going) {
		puttekn("\r\nStorlek: (<xx el. >xx) ",-1);
		if(getstring(EKO,15,NULL)) return(1);
		if(!inmat[0]) break;
		if(inmat[0]!='<' && inmat[0]!='>') {
			puttekn("\r\nDu m�ste skriva '<' eller '>' f�re storleken f�r\r\natt ange mindre respektive st�rre.",-1);
			continue;
		}
		if(inmat[0]=='<') storre=FALSE;
		storlek=atoi(&inmat[1]);
		going=FALSE;
	}
	eka('\n');
	len=strlen(sokstring);
	for(x=0;x<len;x++) sokstring[x]=ToUpper(sokstring[x]);

	for(i=0;i<Servermem->info.areor;i++)
	{
		if(globalSearch)
			areatmp = i;
		else
			areatmp = area2;

		if(!Servermem->areor[areatmp].namn[0] || !arearatt(areatmp, inloggad, &Servermem->inne[nodnr])) continue;
		foundfiles = 0;

		for(filpek=(struct Fil *)Servermem->areor[areatmp].ar_list.mlh_TailPred;
			filpek->f_node.mln_Pred;filpek=(struct Fil *)filpek->f_node.mln_Pred) {
			if((filpek->flaggor & FILE_NOTVALID) && Servermem->inne[nodnr].status < Servermem->cfg.st.filer && filpek->uppladdare != inloggad) continue;
			if(sokstring[0]) {
				if(pat) {
					if(fil) {
						if(!MatchPatternNoCase(token,filpek->namn)) continue;
					} else {
						if(!MatchPatternNoCase(token,filpek->beskr)) continue;
					}
				} else {
					foo=FALSE;
					strcpy(upper,filpek->namn);
					len=strlen(upper);
					for(x=0;x<len;x++) upper[x]=ToUpper(upper[x]);
					if(strstr(upper,sokstring)) foo=TRUE;
					strcpy(upper,filpek->beskr);
					len=strlen(upper);
					for(x=0;x<len;x++) upper[x]=ToUpper(upper[x]);
					if(strstr(upper,sokstring)) foo=TRUE;
					if(!foo) continue;
				}
			}
			if(keys) {
				foo=TRUE;
				for(x=0;x<Servermem->info.nycklar;x++) {
					if(BAMTEST(soknyckel,x) && (!BAMTEST(filpek->nycklar,x))) {
						foo=FALSE;
							break;
					}
				}
				if(!foo) continue;
			}
			if(uppl!=-1 && uppl!=filpek->uppladdare) continue;
			if(storre && filpek->size<storlek) continue;
			else if(!storre && filpek->size>storlek) continue;

			if(!foundfiles)
			{
				sprintf(outbuffer,"\n\rFiler som matchade i arean %s\n\r",Servermem->areor[areatmp].namn);
				puttekn(outbuffer,-1);
				foundfiles = 1;
			}
			found = TRUE;
			ts=localtime(&filpek->tid);
			sprintf(outbuffer,"\r\n%-24s %c%c %7d %02d%02d%02d %-28s %2d\r\n",
				filpek->namn,
				filpek->flaggor & FILE_NOTVALID ? 'V' : ' ',
				filpek->flaggor & FILE_FREEDL ? 'F' : ' ',
					filpek->size,
					ts->tm_year % 100,
					ts->tm_mon+1,
					ts->tm_mday,
					getusername(filpek->uppladdare),
					filpek->downloads);
			if(puttekn(outbuffer,-1)) return(0);
			if(filpek->flaggor & FILE_LONGDESC) sprintf(outbuffer,"**> %s\n",filpek->beskr);
			else sprintf(outbuffer,"    %s\n",filpek->beskr);
			if(puttekn(outbuffer,-1)) return(0);
		}
		if(!globalSearch) break;
	}
	if(!found) puttekn("\n\n\rInga filer hittade\n\r",-1);
	return(0);
}

struct Fil *parsefilallareas(char *skri)
{
	int x;

	struct Fil *fil;

	for(x=0;x<Servermem->info.areor;x++)
	{
		if(!Servermem->areor[x].namn[0] || !arearatt(x, inloggad, &Servermem->inne[nodnr])) continue;
		if(Servermem->areor[x].flaggor & AREA_NODOWNLOAD) continue;

		fil = parsefil(skri, x);
		if(fil) return fil;
	}
}

struct Fil *parsefil(skri,area)
char *skri;
int area;
{
	int quoted = FALSE;
	char tmpstr[42], *pt;
	struct Fil *letpek;
	if(skri[0]==0 || skri[0]==' ') return(NULL);
	if(skri[0]=='"') {
		quoted = TRUE;
		strncpy(tmpstr,&skri[1],41);
		tmpstr[41] = 0;
		if(pt = strchr(tmpstr,'"')) *pt = 0;
	}
	else {
		strncpy(tmpstr,skri,41);
		tmpstr[41] = 0;
	}
	letpek=(struct Fil *)Servermem->areor[area].ar_list.mlh_TailPred;
	while(letpek->f_node.mln_Pred) {
		if((letpek->flaggor & FILE_NOTVALID) &&
		Servermem->inne[nodnr].status < Servermem->cfg.st.filer &&
		letpek->uppladdare != inloggad) {
			letpek=(struct Fil *)letpek->f_node.mln_Pred;
			continue;
		}
		if(quoted) {
			if(!stricmp(tmpstr,letpek->namn)) return(letpek);
		} else {
			if(matchar(tmpstr,letpek->namn)) return(letpek);
		}
		letpek=(struct Fil *)letpek->f_node.mln_Pred;
	}
	return(NULL);
}

void raisefiledl(struct Fil *filpek) {
	long tid;
	time(&tid);
	filpek->downloads++;
	filpek->senast_dl=tid;
	if(updatefile(area2,filpek)) {
		puttekn("\r\n\nKunde inte skriva till datafilen!\r\n",-1);
		return;
	}
}

void filstatus(void) {
	BPTR fh;
	struct Fil *vispek;
	struct tm *ts;
	int x;
	char fullpath[100];
	if(!argument[0]) {
		puttekn("\r\n\nSkriv: Filstatus <filnamn>\r\n",-1);
		return;
	}
	if(!(vispek=parsefil(argument,area2))) {
		puttekn("\r\n\nFinns ingen s�dan fil!\r\n",-1);
		return;
	}
	ts=localtime(&vispek->tid);
	sprintf(outbuffer,"\r\n\nNamn: %s  Fill�ngd: %d  Datum:%4d%02d%02d",
                vispek->namn, vispek->size, ts->tm_year + 1900, ts->tm_mon + 1,
                ts->tm_mday);
	puttekn(outbuffer,-1);
	sprintf(outbuffer,"\r\nUppladdare: %s  Antal downloads %d",getusername(vispek->uppladdare),vispek->downloads);
	puttekn(outbuffer,-1);
	if(vispek->senast_dl) {
          ts=localtime(&vispek->senast_dl);
          sprintf(outbuffer,"\r\nSenaste download: %4d%02d%02d  Statusniv�: %d",
                  ts->tm_year + 1900, ts->tm_mon + 1, ts->tm_mday, vispek->status);
	} else sprintf(outbuffer,"\r\nFilen har aldrig blivit nerladdad.  Statusniv�: %d",vispek->status);
	puttekn(outbuffer,-1);
	if(vispek->flaggor & FILE_NOTVALID) puttekn("\n\rFilen �r inte validerad.",-1);
	if(vispek->flaggor & FILE_FREEDL) puttekn("\n\rFilen har fri download.",-1);
	puttekn("\r\n\nNycklar:\r\n",-1);
	for(x=0;x<Servermem->info.nycklar;x++) {
		if(BAMTEST(vispek->nycklar,x)) {
			sprintf(outbuffer,"%s\r",Servermem->Nyckelnamn[x]);
			puttekn(outbuffer,-1);
		}
	}
	sprintf(outbuffer,"\r\nBeskrivning:\r\n%s\r\n",vispek->beskr);
	puttekn(outbuffer,-1);
	puttekn("\r\n",-1);
	if(!(vispek->flaggor & FILE_LONGDESC)) return;
	sprintf(fullpath,"%slongdesc/%s.long",Servermem->areor[area2].dir[vispek->dir],vispek->namn);
	if(!(fh=Open(fullpath,MODE_OLDFILE))) {
		puttekn("\r\nKunde inte �ppna longdesc-filen!\r\n",-1);
		return;
	}
	while(FGets(fh,outbuffer,100)) {
		if(puttekn(outbuffer,-1)) break;;
		eka('\r');
	}
	Close(fh);
}

void typefil(void) {
	struct Fil *filpek;
	char filnamn[100];
	if(area2==-1) {
		puttekn("\r\n\nDu befinner dig inte i n�gon area!\r\n",-1);
		return;
	}
	if(!argument[0]) {
		puttekn("\r\n\nSkriv: Type <filnamn>\r\n",-1);
		return;
	}
	if(!(filpek=parsefil(argument,area2))) {
		puttekn("\r\n\nFinns ingen s�dan fil!\r\n",-1);
		return;
	}
	if(filpek->status>Servermem->inne[nodnr].status && Servermem->inne[nodnr].status<Servermem->cfg.st.laddaner) {
		puttekn("\r\n\nDu har ingen r�tt att se den filen!\r\n",-1);
		return;
	}
	sprintf(filnamn,"%s%s",Servermem->areor[area2].dir[filpek->dir],filpek->namn);
	puttekn("\r\n\n",-1);
	sendfile(filnamn);
}

void nyafiler(void) {
	int areacnt,len,notvalid=FALSE,headerprinted;
	struct tm *ts;
	struct Fil *sokpek;
	if(argument[0]=='-') notvalid=TRUE;
	for(areacnt=0;areacnt<Servermem->info.areor;areacnt++) {
		if(!arearatt(areacnt, inloggad, &Servermem->inne[nodnr]) || !Servermem->areor[areacnt].namn[0]) continue;
		headerprinted=FALSE;
		for(sokpek=(struct Fil *)Servermem->areor[areacnt].ar_list.mlh_TailPred;sokpek->f_node.mln_Pred;sokpek=(struct Fil *)sokpek->f_node.mln_Pred)
		{
			if(notvalid && !(sokpek->flaggor & FILE_NOTVALID)) continue;
			if((sokpek->flaggor & FILE_NOTVALID) && Servermem->inne[nodnr].status < Servermem->cfg.st.filer && sokpek->uppladdare != inloggad) continue;
			if(sokpek->validtime < Servermem->inne[nodnr].senast_in) continue;
			if(!headerprinted) {
				sprintf(outbuffer,"\r\n\nArean %s\r\n",Servermem->areor[areacnt].namn);
				puttekn(outbuffer,-1);
				len=strlen(outbuffer)-5;
				memset(outbuffer,'-',len);
				outbuffer[len]=0;
				puttekn(outbuffer,-1);
				headerprinted=TRUE;
			}
			ts=localtime(&sokpek->tid);
			sprintf(outbuffer,"\r\n%-24s %c%c %7d %02d%02d%02d %-28s %2d\r\n",
		              sokpek->namn,
		              sokpek->flaggor & FILE_NOTVALID ? 'V' : ' ',
		              sokpek->flaggor & FILE_FREEDL ? 'F' : ' ',
						  sokpek->size,
						  ts->tm_year % 100,
						  ts->tm_mon+1,
						  ts->tm_mday,
						  getusername(sokpek->uppladdare),
						  sokpek->downloads);
			if(puttekn(outbuffer,-1)) return;
			if(sokpek->flaggor & FILE_LONGDESC) sprintf(outbuffer,"**> %s\n",sokpek->beskr);
			else sprintf(outbuffer,"    %s\n",sokpek->beskr);
			if(puttekn(outbuffer,-1)) return;
		}
	}
	puttekn("\r\n",-1);
}

int editkey(char *bitmap) {
	int x,nyckel;
	do {
		if(getstring(EKO,40,NULL)) return(1);
		if(inmat[0]=='?') {
			listnyckel();
			puttekn("'!' f�r att se vilka nycklar du har satt.\r\n",-1);
		} else if(inmat[0]=='!') {
			puttekn("\r\nF�ljande nycklar �r satta:\r\n",-1);
			for(x=0;x<Servermem->info.nycklar;x++) if(BAMTEST(bitmap,x)) {
				puttekn(Servermem->Nyckelnamn[x],-1);
				eka('\r');
			}
		} else if(inmat[0]!=0) {
			if((nyckel=parsenyckel(inmat))==-1) puttekn("\r\nFinns ingen s�dan nyckel\r\n",-1);
			else if(nyckel>=0) {
				sprintf(outbuffer,"\r%s\r",Servermem->Nyckelnamn[nyckel]);
				puttekn(outbuffer,-1);
				if(BAMTEST(bitmap,nyckel)) BAMCLEAR(bitmap,nyckel);
				else BAMSET(bitmap,nyckel);
			}
		}
	} while(inmat[0]!=0);
	return(0);
}

void validerafil(void) {
	struct Fil *valfil;
	if(!argument[0]) {
		puttekn("\n\n\rSkriv: Validera Fil <filnamn>\n\r",-1);
		return;
	}
	valfil=parsefil(argument,area2);
	if(!valfil) {
		puttekn("\n\n\rFinns ingen s�dan fil!\n\r",-1);
		return;
	}
	valfil->flaggor &= ~FILE_NOTVALID;
	valfil->validtime=time(NULL);
	sprintf(outbuffer,"\n\n\rValiderar filen %s\n\r",valfil->namn);
	puttekn(outbuffer,-1);
	if(updatefile(area2,valfil)) {
		puttekn("\r\n\nKunde inte skriva till datafilen!\r\n",-1);
		return;
	}
}

struct Fil *filexists(skri,area)
char *skri;
int area;
{
	struct Fil *letpek;
	if(skri[0]==0 || skri[0]==' ') return(NULL);
	letpek=(struct Fil *)Servermem->areor[area].ar_list.mlh_TailPred;
	while(letpek->f_node.mln_Pred) {
		if(matchar(skri,letpek->namn)) return(letpek);
		letpek=(struct Fil *)letpek->f_node.mln_Pred;
	}
	return(NULL);
}

int valnamn(char *namn,int area,char *textbuf) {
	if(strlen(namn)>21) {
		strcpy(textbuf,"\n\n\rFilnamnet f�r maximalt vara 21 tecken!\n\r");
		return(0);
	}
	if(strpbrk(namn," #?*/:()[]\"")) {
		strcpy(textbuf,"\r\n\nFilnamnet f�r inte inneh�lla tecknen #, ?, *, /, :, (, ), [, ], \"\r\neller mellanslag.\n\r");
		return(0);
	}
	if(filexists(namn,area)) {
		strcpy(textbuf,"\r\n\nFinns redan en fil med det namnet!\r\n");
		return(0);
	}
	return(1);
}

