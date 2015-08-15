#include <exec/types.h>
#include <exec/semaphores.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <utility/tagitem.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include "/Include/NiKomStr.h"
#include "/Include/NiKomLib.h"
#include "NiKomBase.h"
#include "funcs.h"
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <proto/rexxsyslib.h>
#include <rexx/storage.h>

extern char reggadnamn[];
char usernamebuf[50];

extern void __saveds debug_req(char *reqtext,APTR arg);
/* {
        struct EasyStruct myeasy = {
                sizeof(struct EasyStruct),
                0,"NiKomLib Debug","","OK"
        };
        myeasy.es_TextFormat=reqtext;
        EasyRequest(NULL,&myeasy,NULL,arg);
} */

/****** nikom.library/CreateUser ******************************************

    NAME
        CreateUser -- Skapa en ny anv�ndare.

    SYNOPSIS
        error = CreateUser ( nodnummer, taglist)
        d0                   d0			a0
        int CreateUser ( LONG, struct TagList * )

    FUNCTION
        Skapar en ny anv�ndare. Tags/struktur att skicka med, avslutad med
        TAG_DONE.
        
        Dessa tags m�ste anges:
			US_Name         Pekare till namnet p� anv�ndaren.
			US_Street       Pekare till gatan anv�ndaren bor p�.
			US_Address      Pekare till addressen anv�ndaren bor p�.
			US_Country      Pekare till namnet p� landet anv�ndaren kommer
                            ifr�n.
			US_Phonenumber  Pekare till telefonnumret f�r denna anv�ndare.
			US_OtherInfo    Pekare till anv�ndarens annan info f�lt.
			US_Password     Pekare till anv�ndarens l�senord.
							L�senordet krypteras om kryptering
							av l�senord �r p�slaget!
			US_Status       En byte med anv�ndarens status.
			US_Charset      long integer med med aktuellt teckenset.
			US_Prompt		Pekare till anv�ndarens prompt.
			US_Rader		Antal rader anv�ndaren ska f�,
							-1 f�r default antal rader specificerade i
							NiKom:datocfg/System.cfg.
			US_Flags		Vilka flaggor ska s�ttas.
							-1 f�r default flaggorna specificerade i 
							NiKom:datocfg/System.cfg.

    INPUTS
    	nodnummer - Nodnummret anv�ndaren �r inloggad. -1 om anv�ndaren
    				inte �r inloggad p� n�gon nod f�r tillf�llet.
        taglist   - En pekare till taglista.

    RESULT
        error - Ger f�ljande felmeddeladen:
            >=0 - Den skapade anv�ndarens anv�ndarnummer.
            -1  - F�r lite ledigt minne.
            -2  - F�r f� parametrar. (I taglistan)
            -3  - Finns redan en anv�ndare med det namnet.
            -4  - Kunde inte skriva anv�ndaren till disk. (disk full?)
            -5  - Finns ingen anv�ndare inloggad p� det nodnumret.
            -6  - Servern �r inte uppstartad.
        
    EXAMPLE

    NOTES

    BUGS

    SEE ALSO
        DeleteUser(), EditUser(), ReadUser() och SortUserlist()

******************************************************************************
* F�ljande b�r nog nollas: US_Group, US_Brevpek och resten...
*
*/

int __saveds __asm LIBCreateUser(register __d0 LONG nodnummer, register __a0 struct TagItem *taglist, register __a6 struct NiKomBase *NiKomBase)
{
	struct User *newuser;
	struct ShortUser *allokpek;
	struct Mote *motpek=(struct Mote *)NiKomBase->Servermem->mot_list.mlh_Head;
	long tid, anvnummer = -1;
	char dirnamn[100],filnamn[40];
	BPTR lock,fh;
	ULONG tmp;
        struct UnreadTexts unreadTexts;
	
	if(!(newuser=AllocMem(sizeof(struct User),MEMF_CLEAR | MEMF_PUBLIC))) return(-1);

	strcpy(newuser->namn,(char *)GetTagData(US_Name,NULL,taglist));
	if(newuser->namn == NULL)
	{
		FreeMem(newuser,sizeof(struct User));
		return(-2);
	}
	else
		if(NiKomBase->Servermem != NULL)
		{
			if(parsenamn(newuser->namn, NiKomBase) != -1)
			{
				FreeMem(newuser,sizeof(struct User));
				return(-3);
			}
		}
		else
		{
			sprintf(filnamn,"NiKom:Users/0/0/Data");
			if(fh=Open(filnamn, MODE_OLDFILE))
			{
				Close(fh);
				FreeMem(newuser,sizeof(struct User));
				return(-6);
			}
			anvnummer = 0;
		}

	strcpy(newuser->gata,(char *)GetTagData(US_Street,NULL,taglist));
	if(newuser->gata == NULL)
	{
		FreeMem(newuser,sizeof(struct User));
		return(-2);
	}

	strcpy(newuser->postadress,(char *)GetTagData(US_Address,NULL,taglist));
	if(newuser->postadress == NULL)
	{
		FreeMem(newuser,sizeof(struct User));
		return(-2);
	}

	strcpy(newuser->land,(char *)GetTagData(US_Country,NULL,taglist));
	if(newuser->land == NULL)
	{
		FreeMem(newuser,sizeof(struct User));
		return(-2);
	}

	strcpy(newuser->prompt,(char *)GetTagData(US_Prompt,NULL,taglist));
	if(newuser->land == NULL)
	{
		FreeMem(newuser,sizeof(struct User));
		return(-2);
	}

	strcpy(newuser->telefon,(char *)GetTagData(US_Phonenumber,NULL,taglist));
	if(newuser->telefon == NULL)
	{
		FreeMem(newuser,sizeof(struct User));
		return(-2);
	}

	strcpy(newuser->annan_info,(char *)GetTagData(US_OtherInfo,NULL,taglist));
	if(newuser->annan_info == NULL)
	{
		FreeMem(newuser,sizeof(struct User));
		return(-2);
	}

	strcpy(newuser->losen,(char *)GetTagData(US_Password,NULL,taglist));
	if(newuser->losen == NULL)
	{
		FreeMem(newuser,sizeof(struct User));
		return(-2);
	}

	tmp = GetTagData(US_Status,-1,taglist);
	if(tmp == -1)
	{
		FreeMem(newuser,sizeof(struct User));
		return(-2);
	}
	else
	{
		newuser->status = tmp;
	}

	tmp = GetTagData(US_Rader, -2,taglist);
	if(tmp == -2)
	{
		FreeMem(newuser,sizeof(struct User));
		return(-2);
	}
	else
	{
		newuser->rader = tmp;
	}

	tmp = GetTagData(US_Charset,-1,taglist);
	if(tmp == -1)
	{
		FreeMem(newuser,sizeof(struct User));
		return(-2);
	}
	else
	{
		newuser->chrset = tmp;
	}

	tmp = GetTagData(US_Flags,-1,taglist);
	if(tmp == -1)
	{
		FreeMem(newuser,sizeof(struct User));
		return(-2);
	}
	else
	{
		newuser->flaggor = tmp;
	}

	if(newuser->flaggor == -1)
		newuser->flaggor = NiKomBase->Servermem->cfg.defaultflags;

	if(newuser->rader == -1)
		newuser->rader = NiKomBase->Servermem->cfg.defaultrader;

	if(NiKomBase->Servermem->cfg.cfgflags & NICFG_CRYPTEDPASSWORDS)
		strcpy(newuser->losen, LIBCryptPassword(newuser->losen, NiKomBase));

	newuser->tot_tid = 0L;
	newuser->senast_in = 0L;
	newuser->read = 0L;
	newuser->skrivit = 0L;
	newuser->upload = 0;
	newuser->download = 0;
	newuser->loggin = 0;
	newuser->grupper = 0L;
	newuser->defarea = 0L;
	newuser->shell = 0;
	newuser->brevpek = 0;

	memset((void *)newuser->motmed, 0, MAXMOTE/8);
	memset((void *)newuser->motratt, 0, MAXMOTE/8);

	time(&tid);
	newuser->forst_in = tid;

	if(anvnummer == -1)
	{
		anvnummer = ((struct ShortUser *)NiKomBase->Servermem->user_list.mlh_TailPred)->nummer+1;
		newuser->status = NiKomBase->Servermem->cfg.defaultstatus;
		newuser->protokoll = NiKomBase->Servermem->cfg.defaultprotokoll;

		for(;motpek->mot_node.mln_Succ;motpek=(struct Mote *)motpek->mot_node.mln_Succ)
		{
			if(motpek->status & (SKRIVSTYRT | SLUTET)) BAMCLEAR(newuser->motratt,motpek->nummer);
			else BAMSET(newuser->motratt,motpek->nummer);
		}

		if(!(allokpek=(struct ShortUser *)AllocMem(sizeof(struct ShortUser),MEMF_CLEAR | MEMF_PUBLIC)))
		{
			FreeMem(newuser,sizeof(struct User));
			return(-1);
		}
		strcpy(allokpek->namn,newuser->namn);
		allokpek->nummer=anvnummer;
		allokpek->status=newuser->status;
		AddTail((struct List *)&NiKomBase->Servermem->user_list,(struct Node *)allokpek);

		if(nodnummer != -1) memcpy((void *)&NiKomBase->Servermem->inne[nodnummer],(void *)newuser,sizeof(newuser));
	}
	else
	{
		newuser->protokoll = 0;
	}

	LIBLockNiKomBase(NiKomBase);

	sprintf(dirnamn,"NiKom:Users/%d",anvnummer/100);
	if(!(lock=Lock(dirnamn,ACCESS_READ)))
		if(!(lock=CreateDir(dirnamn)))
		{
			FreeMem(newuser,sizeof(struct User));
			return(-4);
		}
	UnLock(lock);
	sprintf(dirnamn,"NiKom:Users/%d/%d",anvnummer/100,anvnummer);
	if(!(lock=Lock(dirnamn,ACCESS_READ)))
		if(!(lock=CreateDir(dirnamn)))
		{
			FreeMem(newuser,sizeof(struct User));
			return(-4);
		}
	UnLock(lock);

	sprintf(filnamn,"NiKom:Users/%d/%d/Data",anvnummer/100,anvnummer);
	if(!(fh=Open(filnamn,MODE_NEWFILE)))
	{
		FreeMem(newuser,sizeof(struct User));
		LIBUnLockNiKomBase(NiKomBase);
		return(-4);
	}
	if(Write(fh,(void *)newuser,sizeof(struct User))==-1) {
		Close(fh);
		FreeMem(newuser,sizeof(struct User));
		LIBUnLockNiKomBase(NiKomBase);
		return(-4);
	}
	Close(fh);

        InitUnreadTexts(&unreadTexts);
        WriteUnreadTexts(&unreadTexts, anvnummer);

	sprintf(filnamn,"Nikom:Users/%d/%d/.firstletter",anvnummer/100,anvnummer);
	if(!(fh=Open(filnamn,MODE_NEWFILE)))
	{
		FreeMem(newuser,sizeof(struct User));
		LIBUnLockNiKomBase(NiKomBase);
		return(-4);
	}
	Write(fh,"0",1);
	Close(fh);
	sprintf(filnamn,"Nikom:Users/%d/%d/.nextletter",anvnummer/100,anvnummer);
	if(!(fh=Open(filnamn,MODE_NEWFILE)))
	{
		FreeMem(newuser,sizeof(struct User));
		LIBUnLockNiKomBase(NiKomBase);
		return(-4);
	}
	Write(fh,"0",1);
	Close(fh);
	LIBUnLockNiKomBase(NiKomBase);
	return(anvnummer);
}
/* End CreateUser */

/****** nikom.library/DeleteUser ******************************************

    NAME
        DeleteUser -- Raderar en anv�ndare.

    SYNOPSIS
        error = DeleteUser( nodnummer, usernumber )
        d0                  d0		   d1
        int DeleteUser( LONG, ULONG )

    FUNCTION
        Raderar en anv�ndare med numret som angivits som argument.

    INPUTS
    	nodnummer	- Nodnummer anv�ndaren �r inloggad p�, -1 om han
    				  inte �r inloggad.

        usernumber	- Anv�ndarnumret f�r anv�ndare att radera.

    RESULT
        error - Ger f�ljande felmeddeladen:
            >=0 - Den raderade anv�ndarens anv�ndarnummer.
            -1  - Kunde inte hitta anv�ndaren som skulle raderas.

    EXAMPLE
        
    NOTES
		OBS! Denna funktion raderar INTE anv�ndarens data-filer, brev etc.
		Det m�ste programmet sj�lv ta hand om.

    BUGS

    SEE ALSO
        DeleteUser(), EditUser(), ReadUser() och SortUserlist()

******************************************************************************
*
*/

int __saveds __asm LIBDeleteUser(register __d0 LONG nodnummer, register __d1 ULONG nummer, register __a6 struct NiKomBase *NiKomBase)
{
	int userfound = 0;
	struct ShortUser *letpek;
	
	for(letpek=(struct ShortUser *)NiKomBase->Servermem->user_list.mlh_Head;letpek->user_node.mln_Succ;letpek=(struct ShortUser *)letpek->user_node.mln_Succ)
		if(letpek->nummer==nummer)
		{
			userfound = 1;
			break;
		}

	if(letpek->user_node.mln_Succ && userfound)
	{
		Remove((struct Node *)letpek);
		
		// ta bort anv�ndaren fr�n Disk ocks�
		// runrexx(15);
		return (int)nummer;
	}
	else
		return -1;
}

/****** nikom.library/EditUser ******************************************

    NAME
        EditUser -- �ndrar en anv�ndares data.

    SYNOPSIS
        error = EditUser( usernumber, taglist )
        d0                d0          a0
        int EditUser (int, struct TagItem * )

    FUNCTION
        �ndrar en befintligt anv�ndare. F�ljande tags kan anv�ndas:
            US_Name         Pekare till namnet p� anv�ndaren.
            US_Street       Pekare till gatan anv�ndaren bor p�.
            US_Address      Pekare till addressen anv�ndaren bor p�.
            US_Country      Pekare till namnet p� landet anv�ndaren kommer
                            ifr�n.
            US_Phonenumber  Pekare till telefonnumret f�r denna anv�ndare.
            US_OtherInfo    Pekare till anv�ndarens annan info f�lt.
            US_Password     Pekare till anv�ndarens l�senord.
            US_Status       En byte med anv�ndarens status.
            US_Prompt		Pekare till anv�ndarens prompt.
            US_Rader        En byte med antal rader f�r anv�ndaren.
            US_Protocol     Anv�ndarens protokoll f�r upload och download.
            US_Tottid       Long integer inneh�llande total tid inloggad.
            US_FirstLogin   Long integer inneh�llande f�rsta login.
            US_LastLogin    Long integer inneh�llande senaste login.
            US_Read         Long integer antalet l�sta texter.
            US_Written      Long integer med antalet skrivna texter.
            US_Flags        Long integer med de aktuella flaggorna f�r
                            denna anv�ndare. (FYLL IN INFO OM FLAGGORNA H�R!)
            US_Defarea      long integer med default area.
            US_Charset      long integer med med aktuellt teckenset.
            US_Uploads      long integer inneh�llande antalet uppladdade
                            filer.
            US_Downloads    long integer inneh�llande antalet nerladdade
                            filer.
            US_Loggin       long integer inneh�llande antalet inloggningar.
            US_Shell        long integer med aktuell nodtyp.

    INPUTS
        taglist     - En pekare till en taglista.
        usernumber  - Anv�ndarens nummer.

    RESULT
        error f�r f�ljande v�rden:
            >=0 - Den �ndrade anv�ndarens anv�ndarnummer.
            -1  - Anv�ndaren finns inte.
            -2  - Anv�ndaren �r raderad.
            -3	- Kunde inte skriva anv�ndaren till disk.

    EXAMPLE

    NOTES

    BUGS

    SEE ALSO
        DeleteUser(), EditUser(), ReadUser() och SortUserlist()

******************************************************************************
*
*/

int __saveds __asm LIBEditUser(register __d0 LONG usernumber, register __a0 struct TagItem *taglist, register __a6 struct NiKomBase *NiKomBase)
{
	int i, userloggedin = -1, cacheduser = -1;
	ULONG tmp;
	struct User user;

	for(i=0;i<MAXNOD;i++)
		if(usernumber==NiKomBase->Servermem->inloggad[i]) break;

	if(i<MAXNOD)
	{
		memcpy(&user,&NiKomBase->Servermem->inne[i],sizeof(struct User));
		userloggedin = i;
	}
	else
	{
/*		for(i=0;i<MAXUSERSCACHED;i++)
			if(NiKomBase->Servermem->UserCache[i].usernumber != -1) break;
		
		if(i<MAXUSERSCACHED)
		{
			memcpy(&user,&NiKomBase->Servermem->UserCache[i].Usercached,sizeof(struct User));
			userloggedin = i;
		}
		else */
			if(readuser(usernumber,&user))
				return(-2);
	}

	if(GetTagData(US_Name,NULL,taglist))
		if(parsenamn((char *)GetTagData(US_Name,NULL,taglist),NiKomBase) == -1)
			strcpy(user.namn,(char *)GetTagData(US_Name,NULL,taglist));

	if(GetTagData(US_Street,NULL,taglist))
		strcpy(user.gata,(char *)GetTagData(US_Street,NULL,taglist));

	if(GetTagData(US_Address,NULL,taglist))
		strcpy(user.postadress,(char *)GetTagData(US_Address,NULL,taglist));

	if(GetTagData(US_Country,NULL,taglist))
		strcpy(user.land,(char *)GetTagData(US_Country,NULL,taglist));

	if(GetTagData(US_Phonenumber,NULL,taglist))
		strcpy(user.telefon,(char *)GetTagData(US_Phonenumber,NULL,taglist));

	if(GetTagData(US_OtherInfo,NULL,taglist))
		strcpy(user.annan_info,(char *)GetTagData(US_OtherInfo,NULL,taglist));

	if(GetTagData(US_Password,NULL,taglist))
		strcpy(user.losen,(char *)GetTagData(US_Password,NULL,taglist));
	
	if(GetTagData(US_Prompt,NULL,taglist))
		strcpy(user.prompt,(char *)GetTagData(US_Prompt,NULL,taglist));

	tmp = GetTagData(US_Status,-1L,taglist);
	if(tmp != -1L)
		user.status = tmp;
		
	tmp = GetTagData(US_Rader,-1L,taglist);
	if(tmp != -1L)
		user.rader = tmp;
	
	tmp = GetTagData(US_Protocol,-1L,taglist);
	if(tmp != -1L)
		user.protokoll = tmp;

	tmp = GetTagData(US_Tottid,-1L,taglist);
	if(tmp != -1L)
		user.tot_tid = tmp;
		
	tmp = GetTagData(US_FirstLogin,-1L,taglist);
	if(tmp != -1L)
		user.forst_in = tmp;
	
	tmp = GetTagData(US_LastLogin,-1L,taglist);	
	if(tmp != -1L)
		user.senast_in = tmp;
		
	tmp = GetTagData(US_Read,-1L,taglist);
	if(tmp != -1L)
		user.read = tmp;
	
	GetTagData(US_Written,-1L,taglist);
	if(tmp != -1L)
		user.skrivit = tmp;
	
	tmp = GetTagData(US_Flags,-1L,taglist);
	if(tmp != -1L)
		user.flaggor = tmp;
	
	tmp = GetTagData(US_Defarea,-1L,taglist);
	if(tmp != -1L)
		user.defarea = tmp;
	
	tmp = GetTagData(US_Charset,-1L,taglist);
	if(tmp != -1L)
		user.chrset = tmp;
	
	tmp = GetTagData(US_Uploads,-1L,taglist);
	if(tmp != -1L)
		user.upload = tmp;
	
	tmp = GetTagData(US_Downloads,-1L,taglist);
	if(tmp != -1L)
		user.download = tmp;
	
	tmp = GetTagData(US_Loggin,-1L,taglist);
	if(tmp != -1L)
		user.loggin = tmp;
	
	tmp = GetTagData(US_Shell,-1L,taglist);
	if(tmp != -1L)
		user.shell = tmp;

/* struct User {
   long tot_tid,forst_in,senast_in,read,skrivit,flaggor,former_textpek,brevpek,
      grupper,defarea,reserv2,chrset,reserv4,reserv5,upload,download,
      loggin,shell;
   char namn[41],gata[41],postadress[41],land[41],telefon[21],
      annan_info[61],losen[16],status,rader,protokoll,
      prompt[6],motratt[MAXMOTE/8],motmed[MAXMOTE/8],vote[MAXVOTE];
}; */


	if(i<MAXNOD)
		memcpy(&NiKomBase->Servermem->inne[i],&user,sizeof(struct User));
	else
	{
		if(writeuser(usernumber,&user)) return(-3);
	}

	return(usernumber);
}

int writeuser(int nummer,struct User *user) {
	BPTR fh;
	char filnamn[40];
	sprintf(filnamn,"NiKom:Users/%d/%d/Data",nummer/100,nummer);
	if(!(fh=Open(filnamn,MODE_NEWFILE))) {
		return(1);
	}
	if(Write(fh,(void *)user,sizeof(struct User))==-1) {
		Close(fh);
		return(1);
	}
	Close(fh);
	return(0);
}

int readuser(int nummer,struct User *user) {
	BPTR fh;
	char filnamn[40];
	sprintf(filnamn,"NiKom:Users/%d/%d/Data",nummer/100,nummer);
	if(!(fh=Open(filnamn,MODE_OLDFILE))) {
		return(1);
	}
	if(Read(fh,(void *)user,sizeof(struct User))==-1) {
		Close(fh);
		return(1);
	}
	Close(fh);
	return(0);
}

/****** nikom.library/ReadUser ******************************************

    NAME
        ReadUser  -- L�ser in information om en anv�ndare.
    SYNOPSIS
        pekare = ReadUser( usernumber, taglist )
        a0                 d0          a0
        void * ReadUser( ULONG, struct TagItem * )

    FUNCTION
        L�ser in information om en anv�ndare, det g�r bra att
        skicka med EN tag h�r.
        
            US_Name         Pekare till namnet p� anv�ndaren.
            US_Street       Pekare till gatan anv�ndaren bor p�.
            US_Address      Pekare till addressen anv�ndaren bor p�.
            US_Country      Pekare till namnet p� landet anv�ndaren kommer
                            ifr�n.
            US_Phonenumber  Pekare till telefonnumret f�r denna anv�ndare.
            US_OtherInfo    Pekare till anv�ndarens annan info f�lt.
            US_Password     Pekare till anv�ndarens l�senord.
            US_Status       En byte med anv�ndarens status.
            US_Prompt		Pekare till anv�ndarens prompt.
            US_Rader        En byte med antal rader f�r anv�ndaren.
            US_Protocol     Anv�ndarens protokoll f�r upload och download.
            US_Tottid       Long integer inneh�llande total tid inloggad.
            US_FirstLogin   Long integer inneh�llande f�rsta login.
            US_LastLogin    Long integer inneh�llande senaste login.
            US_Read         Long integer antalet l�sta texter.
            US_Written      Long integer med antalet skrivna texter.
            US_Flags        Long integer med de aktuella flaggorna f�r
                            denna anv�ndare.
            US_Defarea      long integer med default area.
            US_Charset      long integer med med aktuellt teckenset.
            US_Uploads      long integer inneh�llande antalet uppladdade
                            filer.
            US_Downloads    long integer inneh�llande antalet nerladdade
                            filer.
            US_Loggin       long integer inneh�llande antalet inloggningar.
            US_Shell        long integer med aktuell nodtyp.

    INPUTS
        usernumber      -- Anv�ndarens nummer.
        taglist         -- En pekare till en taglista.

    RESULT

        Returnerar:
        -1 Om tagen inte finns.
        -2 Om anv�ndaren inte existerar.
        
        Annars returneras en pekare med inneh�llet som �nskades.

    EXAMPLE

    NOTES

    BUGS

    SEE ALSO
        DeleteUser(), EditUser(), ReadUser() och SortUserlist()

******************************************************************************
*
*/

void *__saveds __asm LIBReadUser(register __d0 LONG usernumber, register __d1 LONG tag, register __a6 struct NiKomBase *NiKomBase)
{
	int i;
	struct User user;

	for(i=0;i<MAXNOD;i++)
		if(usernumber==NiKomBase->Servermem->inloggad[i]) break;

	if(i<MAXNOD)
	{
		memcpy(&user,&NiKomBase->Servermem->inne[i],sizeof(struct User));
	}
	else
	{
		if(readuser(usernumber,&user))
			return (void *) -2;
	}
	
	switch(tag)
	{
		case US_Name :
			return (void *) user.namn;

		case US_Street :
			return (void *) user.gata;

		case US_Address :
			return (void *) user.postadress;

        case US_Country :
			return (void *) user.land;

        case US_Phonenumber :
			return (void *) user.telefon;

        case US_OtherInfo :
			return (void *) user.annan_info;

        case US_Password :
			return (void *) user.losen;

        case US_Status :
        	return (void *) user.status;

        case US_Prompt :
			return (void *) user.prompt;

        case US_Rader :
        	return (void *) user.rader;

        case US_Protocol :
        	return (void *) user.protokoll;

        case US_Tottid :
        	return (void *) user.tot_tid;

        case US_FirstLogin :
        	return (void *) user.forst_in;

        case US_LastLogin :
        	return (void *) user.senast_in;

        case US_Read :
        	return (void *) user.read;

        case US_Written :
        	return (void *) user.skrivit;

/*      case US_Flags :
        	return (void *) user.flaggor; */

        case US_Defarea :
        	return (void *) user.defarea;

        case US_Charset :
        	return (void *) user.chrset;

        case US_Uploads :
        	return (void *) user.upload;

        case US_Downloads :
        	return (void *) user.download;

        case US_Loggin :
        	return (void *) user.loggin;

        case US_Shell :
        	return (void *) user.shell;
	}

		return (void *) -1;
}
/* struct User {
   long tot_tid,forst_in,senast_in,read,skrivit,flaggor,former_textpek,brevpek,
      grupper,defarea,reserv2,chrset,reserv4,reserv5,upload,download,
      loggin,shell;
   char namn[41],gata[41],postadress[41],land[41],telefon[21],
      annan_info[61],losen[16],status,rader,protokoll,
      prompt[6],motratt[MAXMOTE/8],motmed[MAXMOTE/8],vote[MAXVOTE];
}; */


int parsenamn(skri, NiKomBase)
char *skri;
struct NiKomBase *NiKomBase;
{
	int going=TRUE,going2=TRUE,found=-1, nummer;
	char *faci,*skri2;
	struct ShortUser *letpek;
	if(skri[0]==0 || skri[0]==' ') return(-3);
 	if(isdigit(skri[0]) || (skri[0]=='#' && isdigit(skri[1]))) {
		if(skri[0]=='#') skri++;
		nummer=atoi(skri);
		faci=getusername(nummer, NiKomBase);
		if(!strcmp(faci,"<Raderad Anv�ndare>") || !strcmp(faci,"<Felaktigt anv�ndarnummer>")) return(-1);
		else return(nummer);
	}
	if(matchar(skri,"sysop")) return(0);
	letpek=(struct ShortUser *)NiKomBase->Servermem->user_list.mlh_Head;
	while(letpek->user_node.mln_Succ && going) {
		faci=letpek->namn;
		skri2=skri;
		going2=TRUE;
		if(matchar(skri2,faci)) {
			while(going2) {
				skri2=hittaefter(skri2);
				faci=hittaefter(faci);
				if(skri2[0]==0) {
					found=letpek->nummer;
					going2=going=FALSE;
				}
				else if(faci[0]==0) going2=FALSE;
				else if(!matchar(skri2,faci)) {
					faci=hittaefter(faci);
					if(faci[0]==0 || !matchar(skri2,faci)) going2=FALSE;
				}
			}
		}
		letpek=(struct ShortUser *)letpek->user_node.mln_Succ;
	}
	return(found);
}

char *getusername(int nummer, struct NiKomBase *NiKomBase) {
	struct ShortUser *letpek;
	int found=FALSE;
/*	if(!reggadnamn[0] && nummer>=5) return((char *)0xbadbad42); */
	for(letpek=(struct ShortUser *)NiKomBase->Servermem->user_list.mlh_Head;letpek->user_node.mln_Succ;letpek=(struct ShortUser *)letpek->user_node.mln_Succ) {
		if(letpek->nummer==nummer) {
			if(letpek->namn[0]) sprintf(usernamebuf,"%s #%d",letpek->namn,nummer);
			else strcpy(usernamebuf,"<Raderad Anv�ndare>");
			found=TRUE;
			break;
		}
	}
	if(!found) strcpy(usernamebuf,"<Felaktigt anv�ndarnummer>");
	return(usernamebuf);
}

int matchar(skrivet,facit)
char *skrivet,*facit;
{
	int mat=TRUE,count=0;
	char tmpskrivet,tmpfacit;
	if(facit!=NULL) {
		while(mat && skrivet[count]!=' ' && skrivet[count]!=0) {
			if(skrivet[count]=='*') { count++; continue; }
			tmpskrivet=ToUpper(skrivet[count]);
			tmpfacit=ToUpper(facit[count]);
			if(tmpskrivet!=tmpfacit) mat=FALSE;
			count++;
		}
	}
	return(mat);
}
