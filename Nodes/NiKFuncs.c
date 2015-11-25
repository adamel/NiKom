#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>
#include "NiKomstr.h"
#include "NiKomFuncs.h"
#include "NiKomLib.h"
#include "Logging.h"
#include "Terminal.h"
#include "Cmd_Users.h"
#include "NiKversion.h"

#define EKO             1
#define EJEKO   0
#define KOM             1
#define EJKOM   0

extern struct System *Servermem;
extern int nodnr,inloggad,radcnt,rxlinecount, nodestate;
extern char outbuffer[],inmat[],backspace[],commandhistory[][1024];
extern struct ReadLetter brevread;
extern struct MinList edit_list;

long logintime, extratime;
int mote2,rad,senast_text_typ,felcnt,nu_skrivs,area2,rexxlogout,
        senast_brev_nr,senast_brev_anv,senast_text_nr,senast_text_mote;
char *month[] = { "januari","februari","mars","april","maj","juni",
                "juli","augusti","september","oktober","november","december" },
                *veckodag[] = { "S�ndag","M�ndag","Tisdag","Onsdag","Torsdag","Fredag",
                "L�rdag" },*argument,usernamebuf[50],aliasbuf[1081],argbuf[1081],vilkabuf[50];
struct Header sparhead,readhead;
struct Inloggning Statstr;
struct MinList aliaslist;

int prompt(int kmd) {
        long klockan,tidgrans,tidkvar;
        int parseret,kmdret, after;
        struct Alias *aliaspek;
        struct Kommando *kmdpek;
        if(Servermem->say[nodnr]) displaysay();
        tidgrans=60*(Servermem->cfg.maxtid[Servermem->inne[nodnr].status])+extratime;
        if(Servermem->cfg.maxtid[Servermem->inne[nodnr].status]) {
                if((time(&klockan)-logintime) > tidgrans) {
                        sendfile("NiKom:Texter/TidenSlut.txt");
                        return(-3);
                }
        }
        if(rexxlogout) {
                rexxlogout=FALSE;
                return(-3);
        }
        Servermem->idletime[nodnr] = time(NULL);
        Servermem->action[nodnr] = LASER;
        Servermem->varmote[nodnr] = mote2;
        if(kmd==221) {
                if(Servermem->cfg.ar.nextmeet) sendautorexx(Servermem->cfg.ar.nextmeet);
                puttekn("\r\n(G� till) n�sta m�te ",-1);
        } else if(kmd==210) {
                if(mote2!=-1) {
                        if(Servermem->cfg.ar.nexttext) sendautorexx(Servermem->cfg.ar.nexttext);
                        puttekn("\r\n(L�sa) n�sta text ",-1);
                } else {
                        if(Servermem->cfg.ar.nextletter) sendautorexx(Servermem->cfg.ar.nextletter);
                        puttekn("\r\n(L�sa) n�sta brev ",-1);
                }
        } else if(kmd==211) {
                if(Servermem->cfg.ar.nextkom) sendautorexx(Servermem->cfg.ar.nextkom);
                puttekn("\r\n(L�sa) n�sta kommentar ",-1);
        } else if(kmd==306) {
                Servermem->action[nodnr] = INGET;
                if(Servermem->cfg.ar.setid) sendautorexx(Servermem->cfg.ar.setid);
                puttekn("\r\n(Se) tiden ",-1);
        } else if(kmd==222) {
                if(Servermem->cfg.ar.nextmeet) sendautorexx(Servermem->cfg.ar.nextmeet);
                sprintf(outbuffer,"\r\nG� (till) %s ",Servermem->cfg.brevnamn);
                puttekn(outbuffer,-1);
        }
        tidkvar=logintime+tidgrans-klockan;
        if(tidgrans && tidkvar<300) {
                sprintf(outbuffer,"(%d) %s ",tidkvar/60,Servermem->inne[nodnr].prompt);
                puttekn(outbuffer,-1);
        } else {
                puttekn(Servermem->inne[nodnr].prompt,-1);
                eka(' ');
        }
        if(getstring(EKO,999,NULL)) return(-8);
        if(inmat[0]=='.' || inmat[0]==' ') return(-5);
        if(inmat[0] && (aliaspek=parsealias(inmat)))
        {
			strcpy(aliasbuf,aliaspek->blirtill);
			strcat(aliasbuf," ");
			strncat(aliasbuf,hittaefter(inmat),980);
			strcpy(inmat,aliasbuf);
        }
        if((parseret=parse(inmat))==-1) {
                puttekn("\r\n\nFelaktigt kommando!\r\n",-1);
                if(++felcnt>=2 && !(Servermem->inne[nodnr].flaggor & INGENHELP)) sendfile("NiKom:Texter/2fel.txt");
                return(-5);
        }
        else if(parseret==-3) return(-6);
        else if(parseret==-4) {
                puttekn("\r\n\nDu har ingen r�tt att utf�ra det kommandot!\r\n",-1);
                if(Servermem->cfg.ar.noright) sendautorexx(Servermem->cfg.ar.noright);
                return(-5);
        } else if(parseret==-5) {
                puttekn("\r\n\nFelaktigt l�sen!\r\n",-1);
                return(-5);
        } else {
                kmdpek = getkmdpek(parseret);
                if(!kmdpek) return(-5);
                if(kmdpek->before) sendautorexx(kmdpek->before);
                if(kmdpek->logstr[0]) {
                  LogEvent(USAGE_LOG, INFO, "%s %s",
                           getusername(inloggad), kmdpek->logstr);
                }
                if(kmdpek->vilkainfo[0])
                {
                        Servermem->action[nodnr] = GORNGTANNAT;
                        Servermem->vilkastr[nodnr] = kmdpek->vilkainfo;
                }
                after = kmdpek->after;
                kmdret=dokmd(parseret,kmd);
                if(after) sendautorexx(after);
                return(kmdret);
        }
}

int dokmd(int parseret,int kmd) {
        if(parseret==201) return(ga(argument));
        else if(parseret==210) return(-2);
        else if(parseret==211) return(-4);
        else if(parseret==221) return(-1);
        else if(parseret==222) return(-9);
        else if(parseret==301) return(-3);
        else if(parseret==101) listmot(argument);
        else if(parseret==102) { if(Cmd_ListUsers()) return(-8); }
        else if(parseret==103) listmed();
        else if(parseret==104) sendfile("NiKom:Texter/ListaKommandon.txt");
        else if(parseret==105) listratt();
        else if(parseret==106) listasenaste();
        else if(parseret==107) listnyheter();
        else if(parseret==108) listaarende();
        else if(parseret==109) listflagg();
        else if(parseret==111) listarea();
        else if(parseret==112) listnyckel();
        else if(parseret==113) listfiler();
        else if(parseret==114) listagrupper();
        else if(parseret==115) listgruppmed();
        else if(parseret==116) listabrev();
        else if(parseret==202) { if(skriv()) return(-8); }
        else if(parseret==203) { if(kommentera()) return(-8); }
        else if(parseret==204) { if(personlig()) return(-8); }
        else if(parseret==205) { if(skickabrev()) return(-8); }
        else if(parseret==206) igen();
        else if(parseret==207) atersekom();
        else if(parseret==208) medlem(argument);
        else if(parseret==209) uttrad(argument);
        else if(parseret==212) lasa();
        else if(parseret==213) return(endast());
        else if(parseret==214) {
                if(kmd==211) return(-10);
                else puttekn("\r\n\nFinns inga kommentarer att hoppa �ver!\r\n\n",-1);
        } else if(parseret==215) addratt();
        else if(parseret==216) subratt();
        else if(parseret==217) radtext();
        else if(parseret==218) { if(skapmot()) return(-8); }
        else if(parseret==219) return(radmot());
        else if(parseret==220) var(mote2);
        else if(parseret==223) andmot();
        else if(parseret==224) radbrev();
        else if(parseret==225) rensatexter();
        else if(parseret==226) rensabrev();
        else if(parseret==227) gamlatexter();
        else if(parseret==228) gamlabrev();
        else if(parseret==229) { if(dumpatext()) return(-8); }
        else if(parseret==231) { if(movetext()) return(-8); }
        else if(parseret==232) motesstatus();
        else if(parseret==233) hoppaarende();
        else if(parseret==234) flyttagren();
        else if(parseret==302) sendfile("NiKom:Texter/Help.txt");
        else if(parseret==303) { if(Cmd_ChangeUser()) return(-8); }
        else if(parseret==304) slaav();
        else if(parseret==305) slapa();
        else if(parseret==306) tiden();
        else if(parseret==307) { if(ropa()) return(-8); }
        else if(parseret==308) Cmd_Status();
        else if(parseret==309) Cmd_DeleteUser();
        else if(parseret==310) vilka();
        else if(parseret==311) visainfo();
        else if(parseret==312) getconfig();
        else if(parseret==313) writeinfo();
        else if(parseret==314) { if(sag()) return(-8); }
        else if(parseret==315) { if(skrivlapp()) return(-8); }
        else if(parseret==316) radlapp();
        else if(parseret==317) { grab(); return(-11); }
        else if(parseret==318) { if(skapagrupp()) return(-8); }
        else if(parseret==319) { if(andragrupp()) return(-8); }
        else if(parseret==320) raderagrupp();
        else if(parseret==321) { if(adderagruppmedlem()) return(-8); }
        else if(parseret==322) { if(subtraheragruppmedlem()) return(-8); }
        else if(parseret==323) DisplayVersionInfo();
        else if(parseret==324) alias();
        else if(parseret==325) { nodestate |= NIKSTATE_RELOGIN; return(-3); }
        else if(parseret==326) { if(bytnodtyp()) return(-8); }
        else if(parseret==327) bytteckenset();
        else if(parseret==328) SaveCurrentUser(inloggad, nodnr);
        else if(parseret==401) bytarea();
        else if(parseret==402) filinfo();
        else if(parseret==403) { if(upload()) return(-8); }
        else if(parseret==404) { if(download()) return(-8); }
        else if(parseret==405) { if(skaparea()) return(-8); }
        else if(parseret==406) radarea();
        else if(parseret==407) { if(andraarea()) return(-8); }
        else if(parseret==408) { if(skapafil()) return(-8); }
        else if(parseret==409) radfil();
        else if(parseret==410) { if(andrafil()) return(-8); }
        else if(parseret==411) { if(lagrafil()) return(-8); }
        else if(parseret==412) { if(flyttafil()) return(-8); }
        else if(parseret==413) { if(sokfil()) return(-8); }
        else if(parseret==414) filstatus();
        else if(parseret==415) typefil();
        else if(parseret==416) nyafiler();
        else if(parseret==417) validerafil();
        else if(parseret>=500) return(sendrexx(parseret));
        felcnt=0;
        return(-5);
}

void atersekom(void) {
        struct Mote *motpek;
        if(senast_text_typ==BREV) {
                if(!brevread.reply[0]) puttekn("\r\n\nTexten �r inte n�gon kommentar!\r\n\n",-1);
                else visabrev(atoi(hittaefter(brevread.reply)),atoi(brevread.reply));
        }
        else if(senast_text_typ==TEXT) {
                motpek=getmotpek(senast_text_mote);
                if(motpek->type==MOTE_ORGINAL) {
                        if(readhead.kom_till_nr==-1) puttekn("\r\n\nTexten �r inte n�gon kommentar!\r\n\n",-1);
                        else if(readhead.kom_till_nr<Servermem->info.lowtext) puttekn("\r\n\nTexten finns inte!\r\n\n",-1);
                        else org_visatext(readhead.kom_till_nr);
                }
                else if(motpek->type==MOTE_FIDO) puttekn("\n\n\r�terse Kommenterade kan inte anv�ndas p� Fido-texter\n\r",-1);
        }
        else puttekn("\r\n\nDu har inte l�st n�gon text �nnu!\r\n\n",-1);
}

void igen(void) {
        struct Mote *motpek;
        if(senast_text_typ==BREV) visabrev(senast_brev_nr,senast_brev_anv);
        else if(senast_text_typ==TEXT) {
                motpek=getmotpek(senast_text_mote);
                if(motpek->type==MOTE_ORGINAL) org_visatext(senast_text_nr);
                else if(motpek->type==MOTE_FIDO) fido_visatext(senast_text_nr,motpek);
        }
        else puttekn("\r\n\nDu har inte l�st n�gon text �nnu!\r\n\n",-1);
}

int skriv(void) {
        struct Mote *motpek;
        if(mote2==-1) {
                sprintf(outbuffer,"\r\n\nAnv�nd kommandot 'Brev' i %s.\r\n",Servermem->cfg.brevnamn);
                puttekn(outbuffer,-1);
                return(0);
        }
        motpek=getmotpek(mote2);
        if(!motpek) {
                puttekn("\n\n\rHmm.. M�tet du befinner dig i finns inte.\n\r",-1);
                return(0);
        }
        if(!MayWriteConf(motpek->nummer, inloggad, &Servermem->inne[nodnr])) {
                puttekn("\r\n\nDu f�r inte skriva i det h�r m�tet!\r\n\n",-1);
                return(0);
        }
        if(motpek->type==MOTE_ORGINAL) return(org_skriv());
        else if(motpek->type==MOTE_FIDO) return(fido_skriv(EJKOM,0));
        return(0);
}

int kommentera(void) {
  struct Mote *conf;
  int isCorrect;

  if(argument[0]) {
    if(mote2 == -1) {
      return(brev_kommentera());
    }
    conf = getmotpek(mote2);
    if(conf->type == MOTE_ORGINAL) {
      return(org_kommentera());
    }
    if(conf->type == MOTE_FIDO) {
      if(!MayReplyConf(mote2, inloggad, &Servermem->inne[nodnr])) {
        SendString("\r\n\nDu f�r inte kommentera den texten!\r\n\n");
        return 0;
      }
      if(conf->status & KOMSKYDD) {
        if(GetYesOrNo(
           "\r\n\nVill du verkligen kommentera i ett kommentarsskyddat m�te? ",
           'j', 'n', "Ja\r\n", "Nej\r\n", FALSE, &isCorrect)) {
          return 1;
        }
        if(!isCorrect) {
          return 0;
        }
      }
      return(fido_skriv(KOM, atoi(argument)));
    }
  }

  if(!senast_text_typ) {
    SendString("\n\n\rDu har inte l�st n�gon text �nnu.\n\r");
    return 0;
  }
  if(senast_text_typ == BREV) {
    return(brev_kommentera());
  }
  conf = getmotpek(senast_text_mote);
  if(!conf) {
    LogEvent(SYSTEM_LOG, ERROR,
             "Conference for last read text (confId = %d) does not exist.",
             senast_text_mote);
    DisplayInternalError();
    return 0;
  }
  if(!MayReplyConf(senast_text_mote, inloggad, &Servermem->inne[nodnr])) {
    SendString("\r\n\nDu f�r inte kommentera den texten!\r\n\n");
    return 0;
  }
  if(conf->status & KOMSKYDD) {
    if(GetYesOrNo(
       "\r\n\nVill du verkligen kommentera i ett kommentarsskyddat m�te? ",
       'j', 'n', "Ja\r\n", "Nej\r\n", FALSE, &isCorrect)) {
      return 1;
    }
    if(!isCorrect) {
      return 0;
    }
  }
  if(conf->type == MOTE_ORGINAL) {
    return(org_kommentera());
  }
  if(conf->type == MOTE_FIDO) {
    return(fido_skriv(KOM,senast_text_nr));
  }
  return 0;
}

void lasa(void) {
        int tnr;
        struct Mote *motpek;
        if(!(argument[0]>='0' && argument[0]<='9')) {
                puttekn("\r\n\nSkriv: L�sa <textnr>\r\n\n",-1);
                return;
        }
        tnr=atoi(argument);
        if(mote2==-1) brev_lasa(tnr);
        else {
                motpek=getmotpek(mote2);
                if(!motpek) {
                        puttekn("Hmm.. M�tet du befinner dig i finns inte.\n\r",-1);
                        return;
                }
                if(motpek->type==MOTE_ORGINAL) org_lasa(tnr);
                else if(motpek->type==MOTE_FIDO) fido_lasa(tnr,motpek);
        }
}

void var(int mot) {
        if(mot==-1) varmail();
        else varmote(mot);
}

void varmote(int mote) {
        int cnt;
        struct Mote *motpek;
        motpek = getmotpek(mote);
        sprintf(outbuffer,"\r\n\nDu befinner dig i %s.\r\n",motpek->namn);
        puttekn(outbuffer,-1);
        if(motpek->type == MOTE_FIDO) {
                if(motpek->lowtext > motpek->texter) strcpy(outbuffer,"Det finns inga texter.\n\r");
                else sprintf(outbuffer,"Det finns texter numrerade fr�n %d till %d.\n\r",motpek->lowtext,motpek->texter);
                puttekn(outbuffer,-1);
        }
        cnt=countunread(mote);
        if(!cnt) puttekn("Du har inga ol�sta texter\r\n",-1);
        else if(cnt==1) {
                puttekn("Du har 1 ol�st text\r\n",-1);
        } else {
                sprintf(outbuffer,"Du har %d ol�sta texter\r\n",cnt);
                puttekn(outbuffer,-1);
        }
}

int ga(char *confName) {
  struct UnreadTexts *unreadTexts = &Servermem->unreadTexts[nodnr];
  int confId, becomeMember;
  char buffer[121];
  struct Mote *conf;

  if(matchar(confName, Servermem->cfg.brevnamn)) {
    return -9;
  }
  confId = parsemot(confName);
  if(confId == -3) {
    SendString("\r\n\nSkriv: G� <m�tesnamn>\r\n\n");
    return -5;
  }
  if(confId == -1) {
    SendString("\r\n\nFinns inget s�dant m�te!\r\n\n");
    return -5;
  }

  if(!IsMemberConf(confId, inloggad, &Servermem->inne[nodnr])) {
    conf = getmotpek(confId);
    if(MayBeMemberConf(conf->nummer, inloggad, &Servermem->inne[nodnr])) {

      sprintf(buffer, "\r\n\nDu �r inte medlem i m�tet %s, vill du bli medlem?",
              conf->namn);
      if(GetYesOrNo(buffer, 'j', 'n', "Ja\r\n", "Nej\r\n", TRUE, &becomeMember)) {
        return -8;
      }

      if(becomeMember) {
        BAMSET(Servermem->inne[nodnr].motmed, confId);
        if(conf->type == MOTE_ORGINAL) {
          unreadTexts->lowestPossibleUnreadText[confId] = 0;
        }
        else if(conf->type == MOTE_FIDO) {
          unreadTexts->lowestPossibleUnreadText[confId] = conf->lowtext;
        }
      } else {
        return -5;
      }
    } else {
      return -5;
    }
  }
  return confId;
}

void tiden(void)
{
        long tid,tidgrans;
        struct tm *ts;
        time(&tid);
        ts=localtime(&tid);
        sprintf(outbuffer,"\r\n\nTiden just nu: %s %d %s %d  %d:%02d\r\n",
                veckodag[ts->tm_wday],ts->tm_mday,month[ts->tm_mon],1900+ts->tm_year,
                ts->tm_hour,ts->tm_min);
        puttekn(outbuffer,-1);
        if(Servermem->cfg.maxtid[Servermem->inne[nodnr].status]) {
                tidgrans=60*(Servermem->cfg.maxtid[Servermem->inne[nodnr].status])+extratime;
                sprintf(outbuffer,"\nDin tid tar slut om %d minuter.\r\n",(tidgrans-(tid-logintime))/60);
                puttekn(outbuffer,-1);
        }
}

int skapmot(void) {
  struct ShortUser *shortUser;
  int mad, setPermission, changed, ch, i, fidoDomainId;
  struct FidoDomain *fidoDomain;
  BPTR lock;
  struct User user;
  struct Mote tmpConf,*searchConf,*newConf;

  memset(&tmpConf, 0, sizeof(struct Mote));
  if(argument[0] == '\0') {
    SendString("\r\n\nNamn p� m�tet: ");
    if(GetString(40,NULL)) {
      return 1;
    }
    strcpy(tmpConf.namn, inmat);
  } else {
    strcpy(tmpConf.namn, argument);
  }
  if(parsemot(tmpConf.namn) != -1) {
    SendString("\r\n\nFinns redan ett s�dant m�te!\r\n");
    return 0;
  }
  tmpConf.skapat_tid = time(NULL);;
  tmpConf.skapat_av = inloggad;
  for(;;) {
    SendString("\r\nM�tesAdministrat�r (MAD) : ");
    if(GetString(5,NULL)) {
      return 1;
    }
    if(inmat[0]) {
      if((mad = parsenamn(inmat)) == -1) {
        SendString("\r\nFinns ingen s�dan anv�ndare!");
      } else {
        tmpConf.mad = mad;
        break;
      }
    }
  }
  SendString("\n\rSorteringsv�rde: ");
  tmpConf.sortpri = GetNumber(0, LONG_MAX, NULL);

  if(EditBitFlagShort("\r\nSka m�tet vara slutet?", 'j', 'n', "Slutet", "�ppet",
                     &tmpConf.status, SLUTET)) {
    return 1;
  }
  if(tmpConf.status & SLUTET) {
    SendString("\r\nVilka grupper ska ha tillg�ng till m�tet? (? f�r lista)\r\n");
    if(editgrupp((char *)&tmpConf.grupper)) {
      return 1;
    }
  }
  if(EditBitFlagShort("\r\nSka m�tet vara skrivskyddat?", 'j', 'n',
                     "Skyddat", "Oskyddat", &tmpConf.status, SKRIVSKYDD)) {
    return 1;
  }
  if(EditBitFlagShort("\r\nSka m�tet vara kommentarsskyddat?", 'j', 'n',
                     "Skyddat", "Oskyddat", &tmpConf.status, KOMSKYDD)) {
    return 1;
  }
  if(EditBitFlagShort("\r\nSka m�tet vara hemligt?", 'j', 'n',
                     "Hemligt", "Ej hemligt", &tmpConf.status, HEMLIGT)) {
    return 1;
  }
  if(!(tmpConf.status & SLUTET)) {
    if(EditBitFlagShort("\r\nSka alla anv�ndare bli medlemmar automagiskt?", 'j', 'n',
                       "Ja", "Nej", &tmpConf.status, AUTOMEDLEM)) {
      return 1;
    }
    if(EditBitFlagShort("\r\nSka r�ttigheterna styra skrivm�jlighet?", 'j', 'n',
                       "Ja", "Nej", &tmpConf.status, SKRIVSTYRT)) {
      return 1;
    }
    if(tmpConf.status & SKRIVSTYRT) {
      SendString("\r\nVilka grupper ska ha tillg�ng till m�tet? (? f�r lista)\r\n");
      if(editgrupp((char *)&tmpConf.grupper)) {
        return 1;
      }
    }
  }
  if(EditBitFlagShort("\r\nSka m�tet enbart kommas �t fr�n ARexx?", 'j', 'n',
                     "Ja", "Nej", &tmpConf.status, SUPERHEMLIGT)) {
    return 1;
  }

  SendString("\n\n\rVilken typ av m�te ska det vara?\n\r");
  SendString("1: Lokalt m�te\n\r");
  SendString("2: Fido-m�te\n\n\r");
  SendString("Val: ");
  for(;;) {
    ch = GetChar();
    if(ch == GETCHAR_LOGOUT) {
      return 1;
    }
    if(ch == '1' || ch == '2') {
      break;
    }
  }
  if(ch == '1') {
    SendString("Lokalt m�te\n\n\r");
    tmpConf.type = MOTE_ORGINAL;
  } else {
    SendString("Fido-m�te\n\n\r");
    tmpConf.type = MOTE_FIDO;
    SendString("I vilken katalog ligger .msg-filerna? ");
    if(GetString(79, NULL)) {
      return 1;
    }
    strcpy(tmpConf.dir, inmat);
    if(!(lock = Lock(tmpConf.dir, SHARED_LOCK))) {
      if(!(lock = CreateDir(tmpConf.dir)))
        SendString("\n\rKunde inte skapa katalogen\n\r");
    }
    if(lock) {
      UnLock(lock);
    }
    SendString("\n\rVilket tag-namn har m�tet? ");
    if(GetString(49, NULL)) {
      return 1;
    }
    strcpy(tmpConf.tagnamn, inmat);
    SendString("\n\rVilken origin-rad ska anv�ndas f�r m�tet? ");
    if(GetString(69, Servermem->fidodata.defaultorigin)) {
      return 1;
    }
    strcpy(tmpConf.origin, inmat);
    SendString("\n\n\rVilken teckenupps�ttning ska anv�ndas f�r utg�ende texter?\n\r");
    SendString("1: ISO Latin 1 (ISO 8859-1)\n\r");
    SendString("2: SIS-7 (SF7, 'M�svingar')\n\r");
    SendString("3: IBM CodePage\n\r");
    SendString("4: Mac\n\n\r");
    SendString("Val: ");
    for(;;) {
      ch = GetChar();
      if(ch == GETCHAR_LOGOUT) {
        return 1;
      }
      if(ch == '1' || ch == '2' || ch == '3' || ch == '4') {
        break;
      }
    }
    switch(ch) {
    case '1':
      SendString("ISO Latin 1\n\n\r");
      tmpConf.charset = CHRS_LATIN1;
      break;
    case '2':
      SendString("SIS-7\n\n\r");
      tmpConf.charset = CHRS_SIS7;
      break;
    case '3':
      SendString("IBM CodePage\n\n\r");
      tmpConf.charset = CHRS_CP437;
      break;
    case '4':
      SendString("Mac\n\n\r");
      tmpConf.charset = CHRS_MAC;
      break;
    }
    SendString("Vilken dom�n �r m�tet i?\n\r");
    for(i = 0; i < 10; i++) {
      if(!Servermem->fidodata.fd[i].domain[0]) {
        break;
      }
      SendString("%3d: %s (%d:%d/%d.%d)\n\r",
                 Servermem->fidodata.fd[i].nummer,
                 Servermem->fidodata.fd[i].domain,
                 Servermem->fidodata.fd[i].zone,
                 Servermem->fidodata.fd[i].net,
                 Servermem->fidodata.fd[i].node,
                 Servermem->fidodata.fd[i].point);
    }
    if(i == 0) {
      SendString("\n\rDu m�ste definiera en dom�n i NiKomFido.cfg f�rst!\n\r");
      return 0;
    }
    for(;;) {
      fidoDomainId = GetNumber(0, i - 1, NULL);
      if(fidoDomain = getfidodomain(fidoDomainId, 0)) {
        break;
      } else {
        SendString("\n\rFinns ingen s�dan dom�n.\n\r");
      }
    }
    tmpConf.domain = fidoDomain->nummer;
    SendString("%s\n\n\r", fidoDomain->domain);
  }
  for(i = 0; i < MAXMOTE; i++) {
    if(getmotpek(i) == NULL) {
      break;
    }
  }
  if(i >= MAXMOTE) {
    SendString("\n\n\rDet finns inte plats f�r fler m�ten.\n\r");
    return 0;
  }
  tmpConf.nummer = i;
  if(!(newConf = (struct Mote *)AllocMem(sizeof(struct Mote),
                                         MEMF_CLEAR | MEMF_PUBLIC))) {
    LogEvent(SYSTEM_LOG, ERROR, "Could not allocate %d bytes.", sizeof(struct Mote));
    DisplayInternalError();
    return 0;
  }
  memcpy(newConf, &tmpConf, sizeof(struct Mote));
  ITER_EL(searchConf, Servermem->mot_list, mot_node, struct Mote *) {
    if(searchConf->sortpri > newConf->sortpri) {
      break;
    }
  }
  
  searchConf = (struct Mote *)searchConf->mot_node.mln_Pred;
  Insert((struct List *)&Servermem->mot_list, (struct Node *)newConf,
         (struct Node *)searchConf);
  writemeet(newConf);

  if((newConf->status & AUTOMEDLEM) && !(newConf->status & SKRIVSTYRT)) {
    return 0;
  }
  if(newConf->status & SUPERHEMLIGT) {
    return 0;
  }

  setPermission = (newConf->status & (SLUTET | SKRIVSTYRT)) ? FALSE : TRUE;
  for(i = 0; i < MAXNOD; i++) {
    BAMCLEAR(Servermem->inne[i].motmed, newConf->nummer);
    if(setPermission) {
      BAMSET(Servermem->inne[i].motratt, newConf->nummer);
    } else {
      BAMCLEAR(Servermem->inne[i].motratt, newConf->nummer);
    }
  }

  SendString("\r\n�ndrar i anv�ndardata..\r\n");
  ITER_EL(shortUser, Servermem->user_list, user_node, struct ShortUser *) {
    if(!(shortUser->nummer % 10)) {
      SendString("\r%d", shortUser->nummer);
    }
    if(readuser(shortUser->nummer, &user)) {
      LogEvent(SYSTEM_LOG, ERROR, "Could not read user %d to set "
               "membership/permissions for new conference.", shortUser->nummer);
      DisplayInternalError();
      return 0;
    }
    changed = FALSE;
    if(setPermission != BAMTEST(user.motratt, newConf->nummer)) {
      if(setPermission) {
        BAMSET(user.motratt, newConf->nummer);
      } else {
        BAMCLEAR(user.motratt, newConf->nummer);
      }
      changed = TRUE;
    }
    if(!(newConf->status & AUTOMEDLEM) && BAMTEST(user.motmed, newConf->nummer)) {
      BAMCLEAR(user.motmed, newConf->nummer);
      changed = TRUE;
    }
    if(changed && writeuser(shortUser->nummer, &user)) {
      LogEvent(SYSTEM_LOG, ERROR, "Could not write user %d to set "
               "membership/permissions for new conference.", shortUser->nummer);
      DisplayInternalError();
      return 0;
    }
    
  }
  for(i = 0; i < MAXNOD; i++) {
    BAMCLEAR(Servermem->inne[i].motmed, newConf->nummer);
    if(setPermission) {
      BAMSET(Servermem->inne[i].motratt, newConf->nummer);
    } else {
      BAMCLEAR(Servermem->inne[i].motratt, newConf->nummer);
    }
  }
  BAMSET(Servermem->inne[nodnr].motratt, newConf->nummer);
  BAMSET(Servermem->inne[nodnr].motmed, newConf->nummer);
  if(newConf->type == MOTE_FIDO) {
    ReScanFidoConf(newConf, 0);
  }
  return 0;
}

void listmot(char *foo)
{
        struct Mote *listpek=(struct Mote *)Servermem->mot_list.mlh_Head;
        int cnt, pattern = 0, patret;
        char buffer[61], motpattern[101];
        puttekn("\r\n\n",-1);

        if(foo[0])
        {
			strncpy(buffer,foo,50);
			patret = ParsePatternNoCase(buffer, motpattern, 98);

			if(patret != 1)
			{
				puttekn("\r\n\nDu m�ste ange ett argument som inneh�ller wildcards!\r\n", -1);
				return;
			}

			pattern = 1;
		}

        for(;listpek->mot_node.mln_Succ;listpek=(struct Mote *)listpek->mot_node.mln_Succ) {
                if(!MaySeeConf(listpek->nummer,inloggad,&Servermem->inne[nodnr])) continue;

                if(pattern && !MatchPatternNoCase(motpattern, listpek->namn)) continue;
                cnt=0;
                if(IsMemberConf(listpek->nummer, inloggad, &Servermem->inne[nodnr])) sprintf(outbuffer,"%3d %s ",countunread(listpek->nummer),listpek->namn);
                else sprintf(outbuffer,"   *%s ",listpek->namn);
                if(puttekn(outbuffer,-1)) return;
                if(listpek->status & SLUTET) if(puttekn(" (Slutet)",-1)) return;
                if(listpek->status & SKRIVSKYDD) if(puttekn(" (Skrivskyddat)",-1)) return;
                if(listpek->status & KOMSKYDD) if(puttekn(" (Kom.skyddat)",-1)) return;
                if(listpek->status & HEMLIGT) if(puttekn(" (Hemligt)",-1)) return;
                if(listpek->status & AUTOMEDLEM) if(puttekn(" (Auto)",-1)) return;
                if(listpek->status & SKRIVSTYRT) if(puttekn(" (Skrivstyrt)",-1)) return;
                if((listpek->status & SUPERHEMLIGT) && Servermem->inne[nodnr].status >= Servermem->cfg.st.medmoten) if(puttekn(" (ARexx-styrt)",-1)) return;
                if(puttekn("\r\n",-1)) return;
        }

        if(Servermem->info.hightext > -1)
	        sprintf(outbuffer,"\r\n\nGlobala texter: L�gsta textnummer: %d   H�gsta textnummer: %d\r\n\n",Servermem->info.lowtext, Servermem->info.hightext);
		else
			strcpy(outbuffer, "\r\n\nDet finns inga texter.\r\n\n");
        puttekn(outbuffer,-1);
}

int parse(char *skri) {
        int nummer=0,argtyp, inloggtid;
        char *arg2 = NULL,*ord2;
        struct Kommando *kompek,*forst=NULL;

		inloggtid = time(NULL)-Servermem->inne[nodnr].forst_in;
        if(skri[0]==0) return(-3);
        if(skri[0]>='0' && skri[0]<='9') {
                argument=skri;
                return(212);
        }

		arg2=hittaefter(skri);
        if(isdigit(arg2[0])) argtyp=KOMARGNUM;
        else if(!arg2[0]) argtyp=KOMARGINGET;
        else argtyp=KOMARGCHAR;

        for(kompek=(struct Kommando *)Servermem->kom_list.mlh_Head;kompek->kom_node.mln_Succ;kompek=(struct Kommando *)kompek->kom_node.mln_Succ)
        {
                if(kompek->secret)
                {
                        if(kompek->status > Servermem->inne[nodnr].status) continue;
                        if(kompek->minlogg > Servermem->inne[nodnr].loggin) continue;
                        if(kompek->mindays*86400 > inloggtid) continue;
                        if(kompek->grupper && !(kompek->grupper & Servermem->inne[nodnr].grupper)) continue;
                }
                if(matchar(skri,kompek->namn))
                {
                        ord2=hittaefter(kompek->namn);
                        if((kompek->antal_ord==2 && matchar(arg2,ord2) && arg2[0]) || kompek->antal_ord==1)
                        {
                                if(kompek->antal_ord==1)
                                {
                                        if(kompek->argument==KOMARGNUM && argtyp==KOMARGCHAR) continue;
                                        if(kompek->argument==KOMARGINGET && argtyp!=KOMARGINGET) continue;
                                }
                                if(forst==NULL)
                                {
                                        forst=kompek;
                                        nummer=kompek->nummer;
                                }
                                else if(forst==(struct Kommando *)1L)
                                {
                                        puttekn(kompek->namn,-1);
                                        puttekn("\n\r",-1);
                                }
                                else
                                {
                                        puttekn("\r\n\nFLERTYDIGT KOMMANDO\r\n\n",-1);
                                        puttekn(forst->namn,-1);
                                        puttekn("\n\r",-1);
                                        puttekn(kompek->namn,-1);
                                        puttekn("\n\r",-1);
                                        forst=(struct Kommando *)1L;
                                }
                        }
                }
        }
        if(forst!=NULL && forst!=(struct Kommando *)1L)
        {
			argument=hittaefter(skri);
			if(forst->antal_ord==2) argument=hittaefter(argument);
            memset(argbuf,0,1080);
            strncpy(argbuf,argument,1080);
            argbuf[strlen(argument)]=0;
            argument=argbuf;
        }
        if(forst==NULL) return(-1);
        else if(forst==(struct Kommando *)1L) return(-2);
        else
        {
			if(forst->status > Servermem->inne[nodnr].status || forst->minlogg > Servermem->inne[nodnr].loggin) return(-4);
			if(forst->mindays*86400 > inloggtid) return(-4);
			if(forst->grupper && !(forst->grupper & Servermem->inne[nodnr].grupper)) return(-4);
        }
        if(forst->losen[0])
        {
                puttekn("\r\n\nL�sen: ",-1);
                if(Servermem->inne[nodnr].flaggor & STAREKOFLAG)
	                getstring(STAREKO,20,NULL);
				else
					getstring(EJEKO,20,NULL);
                if(strcmp(forst->losen,inmat)) return(-5);
        }
        return(nummer);
}

int parsemot(char *skri) {
        struct Mote *motpek=(struct Mote *)Servermem->mot_list.mlh_Head;
        int going2=TRUE,found=-1;
        char *faci,*skri2;
        if(skri[0]==0 || skri[0]==' ') return(-3);
        for(;motpek->mot_node.mln_Succ;motpek=(struct Mote *)motpek->mot_node.mln_Succ) {
                if(!MaySeeConf(motpek->nummer, inloggad, &Servermem->inne[nodnr])) continue;
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

void medlem(char *foo) {
  struct UnreadTexts *unreadTexts = &Servermem->unreadTexts[nodnr];
  struct Mote *motpek=(struct Mote *)Servermem->mot_list.mlh_Head;
  BPTR lock;
  int motnr, patret, ant = 0;
  char filnamn[40], motpattern[101], buffer[61];

  if(foo[0]=='-' && (foo[1]=='a' || foo[1]=='A')) {
    for(;motpek->mot_node.mln_Succ;motpek=(struct Mote *)motpek->mot_node.mln_Succ) {
      if(MayBeMemberConf(motpek->nummer, inloggad, &Servermem->inne[nodnr])
         && !IsMemberConf(motpek->nummer, inloggad, &Servermem->inne[nodnr])) {
        BAMSET(Servermem->inne[nodnr].motmed,motpek->nummer);
        if(motpek->type==MOTE_ORGINAL) {
          unreadTexts->lowestPossibleUnreadText[motpek->nummer] = 0;
        }
        else if(motpek->type==MOTE_FIDO) {
          unreadTexts->lowestPossibleUnreadText[motpek->nummer] = motpek->lowtext;
        }
      }
    }
    return;
  }
  /* TK: 970201 wildcard st�d! */
  strncpy(buffer,foo,50);
  patret = ParsePatternNoCase(buffer, motpattern, 98);
  if(patret == -1)
    return;
  else if(patret == 1) {
    puttekn("\r\n\n", -1);
    for(;motpek->mot_node.mln_Succ;motpek=(struct Mote *)motpek->mot_node.mln_Succ) {
      if(MayBeMemberConf(motpek->nummer, inloggad, &Servermem->inne[nodnr])
         && !IsMemberConf(motpek->nummer, inloggad, &Servermem->inne[nodnr])) {
        strncpy(buffer,foo,50);
        ParsePatternNoCase(buffer, motpattern, 98);
        if(MatchPatternNoCase(motpattern, motpek->namn)) {
          ant++;
          sprintf(buffer,"Du �r nu med i m�tet %s\r\n", motpek->namn);
          puttekn(buffer,-1);
          BAMSET(Servermem->inne[nodnr].motmed, motpek->nummer);
          if(motpek->type==MOTE_ORGINAL) {
            unreadTexts->lowestPossibleUnreadText[motpek->nummer] = 0;
          }
          else if(motpek->type==MOTE_FIDO) {
            unreadTexts->lowestPossibleUnreadText[motpek->nummer] = motpek->lowtext;
          }
        }
      }
    }
    if(ant != 1)
      sprintf(buffer,"\r\nDu gick med i totalt %d m�ten\r\n",ant);
    else
      strcpy(buffer,"\r\nDu gick med i ett m�te\r\n");

    puttekn(buffer,-1);
    return;
  }

  motnr=parsemot(foo);
  if(motnr==-3) {
    puttekn("\r\n\nSkriv: Medlem <m�tesnamn>\r\neller: Medlem -a f�r att bli med i alla m�ten.\r\n\n",-1);
    return;
  }
  if(motnr==-1) {
    puttekn("\r\n\nFinns inget s�dant m�te!\r\n\n",-1);
    return;
  }
  if(!MayBeMemberConf(motnr, inloggad, &Servermem->inne[nodnr])) {
    puttekn("\r\n\nDu har inte r�tt att vara med i det m�tet!\r\n\n",-1);
    return;
  }
  BAMSET(Servermem->inne[nodnr].motmed,motnr);
  sprintf(outbuffer, "\r\n\nDu �r nu med i m�tet %s.\r\n\n",getmotnamn(motnr));
  puttekn(outbuffer, -1);
  motpek=getmotpek(motnr);
  if(motpek->type==MOTE_ORGINAL) {
    unreadTexts->lowestPossibleUnreadText[motnr] = 0;
  }
  else if(motpek->type==MOTE_FIDO) {
    unreadTexts->lowestPossibleUnreadText[motnr] = motpek->lowtext;
  }
  sprintf(filnamn,"NiKom:Lappar/%d.motlapp",motnr);
  if(lock=Lock(filnamn,ACCESS_READ)) {
    UnLock(lock);
    sendfile(filnamn);
  }
}

int uttrad(char *foo) {
        struct Mote *motpek=(struct Mote *)Servermem->mot_list.mlh_Head;
        int motnr, ret=-5, patret, ant = 0;
        char motpattern[101], buffer[61];

        if(foo[0]=='-' && (foo[1]=='a' || foo[1]=='A'))
        {
			for(;motpek->mot_node.mln_Succ;motpek=(struct Mote *)motpek->mot_node.mln_Succ)
			{
				if(IsMemberConf(motpek->nummer, inloggad, &Servermem->inne[nodnr]))
					BAMCLEAR(Servermem->inne[nodnr].motmed,motpek->nummer);
			}
			return -1;
        }
										/* TK: 970201 wildcard st�d! */
		strncpy(buffer,foo,50);
		patret = ParsePatternNoCase(buffer, motpattern, 98);

		if(patret == -1)
			return -4;
		else if(patret == 1)
		{
			puttekn("\r\n\n", -1);
			for(;motpek->mot_node.mln_Succ;motpek=(struct Mote *)motpek->mot_node.mln_Succ)
			{
				if(IsMemberConf(motpek->nummer, inloggad, &Servermem->inne[nodnr])
				  && MatchPatternNoCase(motpattern, motpek->namn))
				{
					ant++;
					sprintf(buffer,"Du gick ur m�tet %s\r\n", motpek->namn);
					puttekn(buffer,-1);
					BAMCLEAR(Servermem->inne[nodnr].motmed, motpek->nummer);
				}
			}

			if(ant != 1)
				sprintf(buffer,"\r\nDu gick ur totalt %d m�ten\r\n",ant);
			else
				strcpy(buffer,"\r\nDu gick ur ett m�te\r\n");

			puttekn(buffer,-1);

			return -1;
		}

        motnr=parsemot(foo);
        if(motnr==-3) {
                puttekn("\r\n\nSkriv: Uttr�d <m�tesnamn>\r\n\n",-1);
                return(-5);
        }
        if(motnr==-1) {
                puttekn("\r\n\nFinns inget s�dant m�te!\r\n\n",-1);
                return(-5);
        }
        motpek=getmotpek(motnr);
        if(motpek->status & AUTOMEDLEM) {
                puttekn("\n\n\rDu kan inte uttr�da ur det m�tet!\n\r",-1);
                return(-5);
        }
        if(!IsMemberConf(motnr, inloggad, &Servermem->inne[nodnr])) {
                puttekn("\r\n\nDu �r inte medlem i det m�tet!\r\n\n",-1);
                return(-5);
        }
        if(motnr==mote2) ret=-1;
        BAMCLEAR(Servermem->inne[nodnr].motmed,motnr);
        sprintf(outbuffer,"\r\n\nDu har nu uttr�tt ur m�tet %s.\r\n\n",getmotnamn(motnr));
        puttekn(outbuffer,-1);
        return(ret);
}

int unread(int meet) {
        struct Mote *motpek;
        if(meet==-1) return(checkmail());
        if(!IsMemberConf(meet, inloggad, &Servermem->inne[nodnr])) {
                return(FALSE);
        }
        motpek=getmotpek(meet);
        if(motpek->type==MOTE_ORGINAL) return(checkmote(meet));
        if(motpek->type==MOTE_FIDO) return(checkfidomote(motpek));
        return(0);
}

int countunread(int conf) {
  struct Mote *motpek;
  if(conf == -1) {
    return(countmail(inloggad,Servermem->inne[nodnr].brevpek));
  }
  motpek=getmotpek(conf);
  if(motpek->type==MOTE_ORGINAL) {
    return CountUnreadTexts(conf, &Servermem->unreadTexts[nodnr]);
  }
  if(motpek->type==MOTE_FIDO) {
    return countfidomote(motpek);
  }
  return 0;
}

int nextmeet(int curmeet) {
        struct Mote *motpek;
        int seekmeet=0,mailiscur=FALSE;
        if(checkmail()) return(-1);
        if(curmeet==-1) {
                motpek=(struct Mote *)Servermem->mot_list.mlh_Head;
                mailiscur=TRUE;
        } else {
                motpek=getmotpek(curmeet);
                motpek=(struct Mote *)motpek->mot_node.mln_Succ;
                if(!motpek->mot_node.mln_Succ) motpek=(struct Mote *)Servermem->mot_list.mlh_Head;
        }
        seekmeet=motpek->nummer;
        while(seekmeet!=curmeet) {
                if(!(motpek->status & SUPERHEMLIGT) && unread(seekmeet)) return(seekmeet);
                motpek=(struct Mote *)motpek->mot_node.mln_Succ;
                if(!motpek->mot_node.mln_Succ) {
                        if(mailiscur) return(-2);
                        motpek=(struct Mote *)Servermem->mot_list.mlh_Head;
                }
                seekmeet=motpek->nummer;
        }
        return(-2);
}

int clearmeet(int meet) {
        struct Mote *motpek;
        if(meet==-1) return(mail());
        motpek=getmotpek(meet);
        if(motpek->type==MOTE_ORGINAL) return(clearmote(meet));
        if(motpek->type==MOTE_FIDO) return(clearfidomote(motpek));
        return(-5);
}

void trimLowestPossibleUnreadTextsForFido(void) {
  struct UnreadTexts *unreadTexts = &Servermem->unreadTexts[nodnr];
  struct Mote *motpek;
  ITER_EL(motpek, Servermem->mot_list, mot_node, struct Mote *) {
    if(motpek->type==MOTE_FIDO) {
      if(unreadTexts->lowestPossibleUnreadText[motpek->nummer] < motpek->lowtext) {
        unreadTexts->lowestPossibleUnreadText[motpek->nummer] = motpek->lowtext;
      }
    }
  }
}

int connection(void)
{
        int promret,motret,foo;
        char tellstr[100];
        dellostsay();
        LoadProgramCategory(inloggad);
        NewList((struct List *)&aliaslist);
        trimLowestPossibleUnreadTextsForFido();
        time(&logintime);
        extratime=0;
        memset(&Statstr,0,sizeof(struct Inloggning));
        Statstr.anv=inloggad;
        mote2=-1;
        Servermem->action[nodnr]=GORNGTANNAT;
        strcpy(vilkabuf,"loggar in");
        Servermem->vilkastr[nodnr]=vilkabuf;
        senast_text_typ=0;
        if(Servermem->cfg.logmask & LOG_INLOGG) {
          LogEvent(USAGE_LOG, INFO, "%s loggar in p� nod %d",
                   getusername(inloggad), nodnr);
        }
        sprintf(tellstr,"loggade just in p� nod %d",nodnr);
        tellallnodes(tellstr);
        area2=Servermem->inne[nodnr].defarea;
        if(area2<0 || area2>Servermem->info.areor || !Servermem->areor[area2].namn || !arearatt(area2, inloggad, &Servermem->inne[nodnr])) area2=-1;
        initgrupp();
        rexxlogout=FALSE;
        rxlinecount = TRUE;
        radcnt=0;
        if(Servermem->cfg.ar.postinlogg) sendautorexx(Servermem->cfg.ar.postinlogg);
        DisplayVersionInfo();
        var(mote2);
        for(;;) {
                if(unread(mote2)) {
                        motret=clearmeet(mote2);
                        if(motret==-1) {          /* N�sta M�te */
                                foo=nextmeet(mote2);
                                if(foo==-2) puttekn("\n\n\rFinns inget mer m�te med ol�sta texter!\n\r",-1);
                                else {
                                        mote2=foo;
                                        var(mote2);
                                }
                                continue;
                        }
                        else if(motret==-3) return(0); /* Logga ut */
                        else if(motret==-8) return(1); /* Carriern sl�ppt */
                        else if(motret==-9) {          /* G� till brevl�dan */
                                mote2=-1;
                                var(mote2);
                                continue;
                        }
                        else if(motret==-11) {         /* Endast anropat */
                                var(mote2);
                                continue;
                        }
                        else if(motret>=0) {           /* G� till ngt m�te */
                                mote2=motret;
                                var(mote2);
                                continue;
                        }
                }
                foo=nextmeet(mote2);
                if(foo!=-2) {
                        if(foo==-1) promret=prompt(222);
                        else promret=prompt(221);
                        if(promret==-1 || promret==-6) {  /* G� till m�tet */
                                mote2=foo;
                                var(mote2);
                                continue;
                        }
                        else if(promret==-2) {            /* L�sa n�sta text */
                                puttekn("\n\n\rFinns inga ol�sta texter i detta m�te\n\r",-1);
                                continue;
                        }
                        else if(promret==-3) return(0);
                        else if(promret==-4) {            /* L�sa n�sta kommentar */
                                puttekn("\n\n\rFinns inga fler ol�sta kommentarer\n\r",-1);
                                continue;
                        }
                        else if(promret==-5) continue;    /* Ngt oviktigt kommando givet */
                        else if(promret==-8) return(1);
                        else if(promret==-9) {            /* G� till brevl�dan */
                                mote2=-1;
                                var(mote2);
                                continue;
                        }
                        else if(promret==-11) {           /* Endast anropat */
                                var(mote2);
                                continue;
                        }
                        else if(promret>=0) {             /* G� till ngt m�te */
                                mote2=promret;
                                var(mote2);
                                continue;
                        }
                }
                promret=prompt(306);
                if(promret==-1) puttekn("\r\n\nFinns inga fler m�ten med ol�sta texter\r\n",-1);
                else if(promret==-2) puttekn("\r\n\nFinns inga ol�sta texter i detta m�te\r\n",-1);
                else if(promret==-3) return(0);
                else if(promret==-4) puttekn("\r\n\nFinns inga ol�sta kommentarer\r\n",-1);
                else if(promret==-6) tiden();
                else if(promret==-8) return(1);
                else if(promret==-9) {
                        mote2=-1;
                        var(mote2);
                }
                else if(promret==-11) var(mote2);
                else if(promret>=0) {
                        mote2=promret;
                        var(mote2);
                }
        }
}



/* Temp! timing .. */

void starttimer(unsigned int *start)
{
	timer(start);
}

void endtimer(char *string, unsigned int *startclock)
{
	unsigned int curclock[2];
	unsigned int sekunder;
	int millisekunder;

	timer(curclock);

	sekunder = curclock[0] - startclock[0];
	millisekunder = curclock[1] - startclock[1] - 500;

	while(millisekunder < 0)
	{
		if(sekunder > 0)
		{
			sekunder--;
			millisekunder += 1000000;
		}
		else
			millisekunder = -millisekunder;
	}

	printf("Tid: %u.%d (%s)\n", sekunder, millisekunder, string);
}

