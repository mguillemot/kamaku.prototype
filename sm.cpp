/*
 * hyDDRe
 * $Id: sm.c,v 1.1.1.1 2006-03-31 19:00:44 root Exp $
 */

#include <iostream>
#include <fstream>
#include <stdio.h>
#include "sm.h"

#define DEBUG_USER printf

/*
 * sm
 *
 * @author: Matthieu Guillemot <g@npng.org>
 * @date: 09/06/2005
 * @version: 1.00
 *
 *
 * Lecteur de fichiers SM pour hyDDRe.
 *
 *
 * Format SM :
 *   http://www.stepmania.com/stepmania/viewfile.php?file=README-FIRST.html#The%20.SM%20file%20format
 *
 */

#define XStatus bool
#define XST_SUCCESS true
#define XST_FAILURE false



/*** Affichage de débug ***/



/*** Constantes pour les champs "mandatory" des fetchs génériques ***/

#define MANDATORY   0
#define FACULTATIVE 1


/*** Fonctions internes ***/

/*
 * Renvoie la taille en octets du fichier "filename" stocké sur
 * la carte Compact Flash, ou bien 0 si le fichier n'existe pas.
 */
int get_file_size(char filename[])
{
	std::ifstream infile(filename);
	long size = infile.rdbuf()->pubseekoff(0, std::ios::end, std::ios::in);
	return (int)size;
}

/*
 * Localise la 1è occurence de la sous-chaine "substr" dans le buffer "buf"
 * à partir de la position "pos", puis met à jour "pos" sur la position du
 * premier caractère après la sous-chaine.
 *
 * Renvoie la position du premier caractère correspondant dans "buf", ou bien
 * -1 si la recherche échoue. Dans ce cas, "pos", n'est pas modifié.
 */
int locate_substr(char *buf, char *substr, int *pos)
{
  int j;
  int i = *pos;
  while (buf[i] != 0)
  {
    if (buf[i] == substr[0])
    {
      j = 1;
      while (substr[j] != 0 && buf[i + j] == substr[j])
      {
        j++;
      }
      if (substr[j] == 0)
      {
        *pos = j + i;
        return i;
      }
    }
    i++;
  }
  return -1;
}

/*
 * Cherche le header texte défini par "header_string" dans le buffer "buf".
 * Alloue la mémoire et met le texte éventuellement trouvé dans la chaîne
 * "destination".
 *
 * 2 modes de fonctionnement, définis par "mandatory" :
 *   - MANDATORY : si le header n'est pas trouvé, la fonction renvoie XST_FAILURE
 *   - FACULTATIVE : si le header n'est pas trouvé, la fonction alloue une chaîne
 *     vide dans "destination" et renvoie XST_SUCCESS
 */
XStatus fetch_text_header(char *buf, char *header_string, char **destination, int mandatory)
{
  int i, deb, size;

  i = 0;
  deb = locate_substr(buf, header_string, &i);
  if (deb < 0)
  {
    if (mandatory == MANDATORY)
    {
      return XST_FAILURE;
    }
    *destination = (char *)malloc(1);
    if (*destination == NULL)
    {
      return XST_FAILURE;
    }
    (*destination)[0] = 0; // fin de chaîne
    DEBUG_USER("%s = <not defined>\r\n", header_string);
    return XST_SUCCESS;
  }
  deb = i;
  size = 0;
  while (buf[deb + size] != ';')
  {
    size++;
  }
  *destination = (char *)malloc(size + 1);
  if (*destination == NULL)
  {
    return XST_FAILURE;
  }
  for (i = 0; i < size; i++)
  {
    (*destination)[i] = buf[deb + i];
  }
  (*destination)[size] = 0; // fin de chaîne
  DEBUG_USER("%s = %s\r\n", header_string, *destination);
  return XST_SUCCESS;
}

/*
 * Cherche le header booléen défini par "header_string" dans le buffer "buf".
 * Met la valeur éventuellement trouvée dans "destination".
 *
 * La valeur True est représentée par la chaîne "YES".
 * La valeur False est représentée par la chaîne "NO".
 * Toute autre chaîne est considérée comme invalide.
 *
 * 2 modes de fonctionnement, définis par "mandatory" :
 *   - MANDATORY : si le header n'est pas trouvé, la fonction renvoie XST_FAILURE
 *   - FACULTATIVE : si le header n'est pas trouvé, la fonction met 0
 *     dans "destination" et renvoie XST_SUCCESS
 */
XStatus fetch_boolean_header(char *buf, char *header_string, int *destination, int mandatory)
{
  int i, deb, size;

  i = 0;
  deb = locate_substr(buf, header_string, &i);
  if (deb < 0)
  {
    if (mandatory == MANDATORY)
    {
      return XST_FAILURE;
    }
    *destination = 0;
    DEBUG_USER("%s = <not defined>\r\n", header_string);
    return XST_SUCCESS;
  }
  deb = i;
  if (buf[deb] == 'Y' && buf[deb + 1] == 'E' && buf[deb + 2] == 'S')
  {
    *destination = 1;
  }
  else if (buf[deb] == 'N' && buf[deb + 1] == 'O')
  {
    *destination = 0;
  }
  else
  {
    return XST_FAILURE;
  }
  if (*destination)
  {
    DEBUG_USER("%s = YES\r\n", header_string);
  }
  else
  {
    DEBUG_USER("%s = NO\r\n", header_string);
  }
  return XST_SUCCESS;
}

/*
 * Affiche un nombre flottant sur la sortie standard.
 */
void print_float(float f)
{
  float uf, d;
  uf = (f < 0) ? -f : f;
  d = uf - (float)((int)uf);
  d *= 1000;
  if (f >= 0)
  {
    DEBUG_USER("%d.%d", (int)uf, (int)d);
  }
  else
  {
    DEBUG_USER("-%d.%d", (int)uf, (int)d);
  }
}

/*
 * Lit un nombre flottant dans le buffer "buf" à partir de la position "pos"
 * et le renvoie, tout en mettant dans "pos" la position du 1er caractère du buffer
 * n'appartenant plus au flottant.
 *
 * Prérequis :
 *   "pos" pointe bien sur un nombre flottant.
 */
float read_float(char *buf, int *pos)
{
  int sign, digit, i;
  float dividor;
  float res = 0.0;

  i = *pos;
  if (buf[i] == '-')
  {
    sign = -1;
    i++;
  }
  else
  {
    sign = 1;
  }
  while (buf[i] != '.' && buf[i] >= '0' && buf[i] <= '9')
  {
    digit = (buf[i] - '0');
    res = 10 * res + (float)digit;
    i++;
  }
  if (buf[i] == '.')
  {
    i++; // on saute le point
    dividor = 0.1;
    while (buf[i] >= '0' && buf[i] <= '9')
    {
      digit = (buf[i] - '0');
      res += dividor * (float)digit;
      dividor *= 0.1;
      i++;
    }
  }
  *pos = i;
  return ((sign == 1) ? res : -res);
}

/*
 * Cherche le header flottant défini par "header_string" dans le buffer "buf".
 * Met la valeur éventuellement trouvée dans "destination".
 *
 * 2 modes de fonctionnement, définis par "mandatory" :
 *   - MANDATORY : si le header n'est pas trouvé, la fonction renvoie XST_FAILURE
 *   - FACULTATIVE : si le header n'est pas trouvé, la fonction met 0.0
 *     dans "destination" et renvoie XST_SUCCESS
 */
XStatus fetch_float_header(char *buf, char *header_string, float *destination, int mandatory)
{
  int i, j;

  j = 0;
  i = locate_substr(buf, header_string, &j);
  if (i < 0)
  {
    if (mandatory == MANDATORY)
    {
      return XST_FAILURE;
    }
    *destination = 0.0;
    DEBUG_USER("%s = <not defined>\r\n", header_string);
    return XST_SUCCESS;
  }
  i = j;
  *destination = read_float(buf, &i);
  DEBUG_USER("%s = ", header_string);
  print_float(*destination);
  DEBUG_USER("\r\n");
  return XST_SUCCESS;
}

/*
 * Lit le champ #BPMS dans le buffer "buf" et enrichit "sm" avec les
 * informations lues.
 */
XStatus fetch_bpms(char *buf, SMfile *sm)
{
  int i, i2, j;

  j = 0;
  i = locate_substr(buf, "#BPMS:", &j);
  if (i < 0)
  {
    return XST_FAILURE;
  }
  i = i2 = j;

  // Calcul du nombre d'entrées
  sm->bpms_nb = 0;
  while (buf[i2] != ';')
  {
    read_float(buf, &i2);
    i2++;
    read_float(buf, &i2);
    sm->bpms_nb++;
    while (buf[i2] != ',' && buf[i2] != ';')
    {
      i2++;
    }
    if (buf[i2] == ',')
    {
      i2++;
    }
  }
  if (sm->bpms_nb == 0)
  {
    return XST_SUCCESS;
  }

  // Lecture des entrées
  sm->bpms = (BPMchange *)malloc(sm->bpms_nb * sizeof(BPMchange));
  if (sm->bpms == NULL)
  {
    return XST_FAILURE;
  }
  DEBUG_USER("#BPMS: =\r\n");
  for (j = 0; j < sm->bpms_nb; j++)
  {
    DEBUG_USER("  ");
    sm->bpms[j].beat = read_float(buf, &i);
    print_float(sm->bpms[j].beat);
    DEBUG_USER("=");
    i++; // pour zapper le '='
    sm->bpms[j].bpm = read_float(buf, &i);
    print_float(sm->bpms[j].bpm);
    while (buf[i] != ',' && buf[i] != ';')
    {
      i++;
    }
    if (buf[i] == ',')
    {
      i++;
    }
    DEBUG_USER("\r\n");
  }

  return XST_SUCCESS;
}

/*
 * Lit le champ #STOPS dans le buffer "buf" et enrichit "sm" avec les
 * informations lues.
 */
XStatus fetch_stops(char *buf, SMfile *sm)
{
  int i, i2, j;

  j = 0;
  i = locate_substr(buf, "#STOPS:", &j);
  if (i < 0)
  {
    return XST_FAILURE;
  }
  i2 = i = j;

  // Calcul du nombre d'entrées
  sm->stops_nb = 0;
  while (buf[i2] != ';')
  {
    read_float(buf, &i2);
    i2++;
    read_float(buf, &i2);
    sm->stops_nb++;
    while (buf[i2] != ',' && buf[i2] != ';')
    {
      i2++;
    }
    if (buf[i2] == ',')
    {
      i2++;
    }
  }
  if (sm->stops_nb == 0)
  {
    return XST_SUCCESS;
  }

  // Lecture des entrées
  sm->stops = (Stop *)malloc(sm->stops_nb * sizeof(Stop));
  if (sm->stops == NULL)
  {
    return XST_FAILURE;
  }
  DEBUG_USER("#STOPS: =\r\n");
  for (j = 0; j < sm->stops_nb; j++)
  {
    DEBUG_USER("  ");
    sm->stops[j].beat = read_float(buf, &i);
    print_float(sm->stops[j].beat);
    DEBUG_USER("=");
    i++; // pour zapper le '='
    sm->stops[j].sec = read_float(buf, &i);
    print_float(sm->stops[j].sec);
    while (buf[i] != ',' && buf[i] != ';')
    {
      i++;
    }
    if (buf[i] == ',')
    {
      i++;
    }
    DEBUG_USER("\r\n");
  }

  return XST_SUCCESS;
}

/*
 * "c" est-il un caractère alphanumérique ?
 */
int is_alnum(char c)
{
  if (c >= 'a' && c <= 'z')
    return 1;
  if (c >= 'A' && c <= 'Z')
    return 1;
  if (c >= '0' && c <= '9')
    return 1;
  return 0;
}

/*
 * Avance jusqu'au prochain caractère alphanumérique dans "buf" depuis "pos",
 * ou bien jusqu'à la fin du buffer. Si "pos" pointe déjà sur un caractère
 * alphénumérique, il ne bouge pas.
 */
void to_next_alnum(char *buf, int *pos)
{
  int i;

  i = *pos;
  while ((! is_alnum(buf[i])) && buf[i] != 0)
  {
    i++;
  }
  *pos = i;
}

/*
 * Recopie dans un buffer alloué sur mesure la sous-chaîne à partir de la position "pos"
 * dans le buffer "buf" jusqu'à la fin du buffer ou bien la 1è occurence
 * de "token". Positionne "pos" sur le token ou bien la fin du buffer.
 */
char *fetch_string_until_token(char *buf, int *pos, char token)
{
  int i, j, size;
  char *res;

  i = j = *pos;
  size = 0;
  while (buf[i] != 0 && buf[i] != token)
  {
    i++;
    size++;
  }
  res = (char *)malloc(size + 1);
  if (res == NULL)
  {
    return NULL;
  }
  for (i = 0; i < size; i++)
  {
    res[i] = buf[j];
    j++;
  }
  res[size] = 0; // fin de chaîne
  *pos = j;
  return res;
}

/*
 * Convertit un caractère en minuscule.
 */
char lowercase(char c)
{
  if (c >= 'A' && c <= 'Z')
    return c - ('A' - 'a');
  return c;
}

/*
 * Teste l'égalité de 2 chaînes de caractères sans tenir compte
 * de la casse.
 */
int str_equal(char *a, char *b)
{
  int i = 0;
  while (a[i] != 0 && b[i] != 0)
  {
    if (lowercase(a[i]) != lowercase(b[i]))
    {
      return 0;
    }
    i++;
  }
  if (a[i] == b[i]) // == 0
  {
    return 1;
  }
  return 0;
}

/*
 * Lit tous les headers des champs #NOTES dans le buffer "buf"
 * et enrichit "sm" avec les informations lues.
 */
XStatus fetch_notes(char *buf, SMfile *sm)
{
  int deb, i, j, k;
  float f;
  char *s;

  // Calcul du nombre d'entrées
  i = 0;
  sm->level_nb = 0;
  do
  {
    deb = locate_substr(buf, "#NOTES:", &i);
    if (deb > 0)
    {
      sm->level_nb++;
    }
  } while (deb >= 0);

  // Lecture des entrées
  sm->level = (Level *)malloc(sm->level_nb * sizeof(Level));
  if (sm->level == NULL)
  {
    return XST_FAILURE;
  }
  DEBUG_USER("#NOTES: =\r\n");
  i = 0;
  for (j = 0; j < sm->level_nb; j++)
  {
    DEBUG_USER("  ");
    locate_substr(buf, "#NOTES:", &i);

    // champ "notestype"
    to_next_alnum(buf, &i);
    s = fetch_string_until_token(buf, &i, ':');
    if (str_equal(s, "dance-single"))
    {
      sm->level[j].notestype = DANCE_SINGLE;
      DEBUG_USER("dance-single | ");
    }
    else
    {
      sm->level[j].notestype = OTHER;
      DEBUG_USER("other | ");
    }
  i++; // on zappe le ':'
    free(s);

    // champ "description"
  k = i;
    to_next_alnum(buf, &i);
  locate_substr(buf, ":", &k);
  if (i > k)
  {
    // description vide
    sm->level[j].description = (char *)malloc(1);
      if (sm->level[j].description == NULL)
    {
      return XST_FAILURE;
    }
      sm->level[j].description[0] = 0; // fin de chaîne
    DEBUG_USER("<pas de description> | ");
    }
  else
  {
      sm->level[j].description = fetch_string_until_token(buf, &i, ':');
      DEBUG_USER("%s | ", sm->level[j].description);
    }

    // champ "difficultyclass"
    to_next_alnum(buf, &i);
    s = fetch_string_until_token(buf, &i, ':');
    if (str_equal(s, "BEGINNER"))
    {
      sm->level[j].difficultyclass = BEGINNER;
      DEBUG_USER("beginner | ");
    }
    else if (str_equal(s, "EASY"))
    {
      sm->level[j].difficultyclass = EASY;
      DEBUG_USER("easy | ");
    }
    else if (str_equal(s, "MEDIUM"))
    {
      sm->level[j].difficultyclass = MEDIUM;
      DEBUG_USER("medium | ");
    }
    else if (str_equal(s, "HARD"))
    {
      sm->level[j].difficultyclass = HARD;
      DEBUG_USER("hard | ");
    }
    else if (str_equal(s, "CHALLENGE"))
    {
      sm->level[j].difficultyclass = CHALLENGE;
      DEBUG_USER("challenge | ");
    }
    else
    {
      return XST_FAILURE;
    }
    free(s);

    // champ "difficultymeter"
    to_next_alnum(buf, &i);
    f = read_float(buf, &i);
    sm->level[j].difficultymeter = (int)f;
    if (sm->level[j].difficultymeter <= 0 || sm->level[j].difficultymeter > 13)
    {
      return XST_FAILURE;
    }
    DEBUG_USER("%d | ", sm->level[j].difficultymeter);

    // champ "difficultygrade"
    to_next_alnum(buf, &i);
    for (k = 0; k < 5; k++)
    {
      sm->level[j].difficultygrade[k] = read_float(buf, &i);
      to_next_alnum(buf, &i);
      print_float(sm->level[j].difficultygrade[k]);
      if (k < 4)
      {
        DEBUG_USER(",");
      }
    }

    // champ "measure" non-lu ici
    // on stocke dans "measure_nb" la position du 1er caractère des mesures
    sm->level[j].measure_nb = i;
    DEBUG_USER("\r\n");
  }

  return XST_SUCCESS;
}

/*
 * Remplace tous les commentaires commencant par // dans le buffer par
 * des espaces.
 */
void strip_comments(char *buf)
{
  int i, clr;

  clr = 0;
  i = 0;
  while (buf[i] != 0)
  {
    if (buf[i] == '\n')
    {
      clr = 0;
    }
    else if (clr)
    {
      buf[i] = ' ';
    }
    else if (buf[i] == '/' && buf[i + 1] == '/')
    {
      buf[i] = ' ';
      clr = 1;
    }
    i++;
  }
}

/*
 * Convertit un caractère en TapType
 */
TapType char_to_TapType(char c)
{
  if (c == '1')
    return TAP;
  if (c == '2')
    return BEGIN_HOLD;
  if (c == '3')
    return END_HOLD;
  return NONE;
}

/*
 * Récupère dans le buffer "buf" une mesure qui commence à la position "pos"
 * et la stocke dans "destination" qui doit avoir été alloué auparavant
 * tout en positionnant "pos" sur le premier caractère à la fin de la mesure.
 */
XStatus fetch_measure(char *buf, int *pos, Measure *destination)
{
  int i, j, k;

  // Calcul du nombre de notes
  destination->notes_nb = 0;
  i = *pos;
  while (buf[i] != ',' && buf[i] != ';')
  {
    if (buf[i] == '\n')
    {
      destination->notes_nb++;
    }
    i++;
  }
  if (buf[i] == ';' && buf[i - 1] != '\n')
  {
    destination->notes_nb++;
  }

  // Récupération des notes
  destination->notes = (Tap *)malloc(destination->notes_nb * sizeof(Tap));
  if (destination->notes == NULL)
  {
    return XST_FAILURE;
  }
  i = *pos;
  for (k = 0; k < destination->notes_nb; k++)
  {
    for (j = 0; j < 4; j++)
    {
      destination->notes[k].tap[j] = char_to_TapType(buf[i]);
      i++;
    }
    to_next_alnum(buf, &i);
  }
  *pos = i;
  return XST_SUCCESS;
}

XStatus load_level(char *buf, SMfile *sm, int level)
{
  int i, k, deb, fin;
  XStatus status;
  Level *lvl;

  // On fetche le level
  DEBUG_USER("Fetching level %d...\r\n", level);
  if (level < 0 || level >= sm->level_nb)
  {
    DEBUG_USER("ERROR: %d is not a valid level number !\r\n", level);
    return XST_FAILURE;
  }
  lvl = &(sm->level[level]);
  if (lvl->notestype != DANCE_SINGLE)
  {
    DEBUG_USER("SKIP: dance-single is the only supported mode.\r\n");
    return XST_SUCCESS;
  }

  // Zone de début et fin des notes
  deb = i = lvl->measure_nb;
  fin = locate_substr(buf, ";", &i);

  // Nombre de mesures
  lvl->measure_nb = 1;
  i = deb;
  do
  {
    k = locate_substr(buf, ",", &i);
    if (k >= 0 && i <= fin)
    {
      lvl->measure_nb++;
    }
  } while (k >= 0 && i <= fin);
  DEBUG_USER("Fetching %d measures: ", lvl->measure_nb);

  // Récupération des mesures
  lvl->measure = (Measure *)malloc(lvl->measure_nb * sizeof(Measure));
  if (lvl->measure == NULL)
  {
    DEBUG_USER("ERROR: not enough memory for measures\r\n");
    return XST_FAILURE;
  }
  i = deb;
  for (k = 0; k < lvl->measure_nb; k++)
  {
    status = fetch_measure(buf, &i, &(lvl->measure[k]));
    if (status == XST_SUCCESS)
    {
      DEBUG_USER("+");
    }
    else
    {
      DEBUG_USER("\r\nERROR: unable to fetch measure %d\r\n", k);
      return XST_FAILURE;
    }
    to_next_alnum(buf, &i);
  }
  DEBUG_USER(" OK !\r\n");

  DEBUG_USER("Fetching level %d... done.\r\n", level);
  return XST_SUCCESS;
}

XStatus load_sm(SMfile *sm, char filename[])
{
  char *buf;
  int filesize, i, k;
  XStatus status;
  FILE *infile;

  // On calcule la taille du fichier
  filesize = get_file_size(filename);
  if (filesize <= 0)
  {
    return XST_FAILURE;
  }

  // On charge tout le fichier dans un énooooorme buffer (vive la subtilité !)
  DEBUG_USER("Loading file %s in internal buffer... ", filename);
  buf = (char *)malloc(filesize + 1);
  if (buf == NULL)
  {
    DEBUG_USER("not enough memory !\r\n");
    return XST_FAILURE;
  }
  infile = fopen(filename, "r");
  if (! infile)
  {
    DEBUG_USER("file not found !\r\n");
    return XST_FAILURE;
  }
  fread(buf, 1, filesize, infile);
  fclose(infile);
  buf[filesize] = 0; // marqueur de fin de buffer
  DEBUG_USER("done.\r\n");

  // On remplace tous les commentaires par des espaces
  strip_comments(buf);

  // On remplit les headers
  DEBUG_USER("Fetching headers...\r\n");
  status = fetch_text_header(buf, "#TITLE:", &(sm->title), MANDATORY);
  if (status == XST_FAILURE)
  {
    DEBUG_USER("Error trying to fetch #TITLE\r\n");
    return XST_FAILURE;
  }
  status = fetch_text_header(buf, "#SUBTITLE:", &(sm->subtitle), MANDATORY);
  if (status == XST_FAILURE)
  {
    DEBUG_USER("Error trying to fetch #SUBTITLE\r\n");
    return XST_FAILURE;
  }
  status = fetch_text_header(buf, "#ARTIST:", &(sm->artist), MANDATORY);
  if (status == XST_FAILURE)
  {
    DEBUG_USER("Error trying to fetch #ARTIST\r\n");
    return XST_FAILURE;
  }
  status = fetch_text_header(buf, "#TITLETRANSLIT:", &(sm->titletranslit), FACULTATIVE);
  if (status == XST_FAILURE)
  {
    DEBUG_USER("Error trying to fetch #TITLETRANSLIT\r\n");
    return XST_FAILURE;
  }
  status = fetch_text_header(buf, "#SUBTITLETRANSLIT:", &(sm->subtitletranslit), FACULTATIVE);
  if (status == XST_FAILURE)
  {
    DEBUG_USER("Error trying to fetch #SUBTITLETRANSLIT\r\n");
    return XST_FAILURE;
  }
  status = fetch_text_header(buf, "#ARTISTTRANSLIT:", &(sm->artisttranslit), FACULTATIVE);
  if (status == XST_FAILURE)
  {
    DEBUG_USER("Error trying to fetch #ARTISTTRANSLIT\r\n");
    return XST_FAILURE;
  }
  status = fetch_text_header(buf, "#CREDIT:", &(sm->credit), FACULTATIVE);
  if (status == XST_FAILURE)
  {
    DEBUG_USER("Error trying to fetch #CREDIT\r\n");
    return XST_FAILURE;
  }
  status = fetch_text_header(buf, "#BANNER:", &(sm->banner), FACULTATIVE);
  if (status == XST_FAILURE)
  {
    DEBUG_USER("Error trying to fetch #BANNER\r\n");
    return XST_FAILURE;
  }
  status = fetch_text_header(buf, "#BACKGROUND:", &(sm->background), FACULTATIVE);
  if (status == XST_FAILURE)
  {
    DEBUG_USER("Error trying to fetch #BACKGROUND\r\n");
    return XST_FAILURE;
  }
  status = fetch_text_header(buf, "#CDTITLE:", &(sm->cdtitle), FACULTATIVE);
  if (status == XST_FAILURE)
  {
    DEBUG_USER("Error trying to fetch #CDTITLE\r\n");
    return XST_FAILURE;
  }
  status = fetch_text_header(buf, "#MUSIC:", &(sm->music), MANDATORY);
  if (status == XST_FAILURE)
  {
    DEBUG_USER("Error trying to fetch #MUSIC\r\n");
    return XST_FAILURE;
  }
  status = fetch_float_header(buf, "#OFFSET:", &(sm->offset), MANDATORY);
  if (status == XST_FAILURE)
  {
    DEBUG_USER("Error trying to fetch #OFFSET\r\n");
    return XST_FAILURE;
  }
  status = fetch_float_header(buf, "#SAMPLESTART:", &(sm->samplestart), MANDATORY);
  if (status == XST_FAILURE)
  {
    DEBUG_USER("Error trying to fetch #SAMPLESTART\r\n");
    return XST_FAILURE;
  }
  status = fetch_float_header(buf, "#SAMPLELENGTH:", &(sm->samplelength), MANDATORY);
  if (status == XST_FAILURE)
  {
    DEBUG_USER("Error trying to fetch #SAMPLELENGTH\r\n");
    return XST_FAILURE;
  }
  status = fetch_boolean_header(buf, "#SELECTABLE:", &(sm->selectable), FACULTATIVE);
  if (status == XST_FAILURE)
  {
    DEBUG_USER("Error trying to fetch #SELECTABLE\r\n");
    return XST_FAILURE;
  }
  status = fetch_bpms(buf, sm);
  if (status == XST_FAILURE)
  {
    DEBUG_USER("Error trying to fetch #BPMS\r\n");
    return XST_FAILURE;
  }
  status = fetch_stops(buf, sm);
  if (status == XST_FAILURE)
  {
    DEBUG_USER("Error trying to fetch #STOPS\r\n");
    return XST_FAILURE;
  }
  status = fetch_notes(buf, sm);
  if (status == XST_FAILURE)
  {
    DEBUG_USER("Error trying to fetch #NOTES\r\n");
    return XST_FAILURE;
  }
  DEBUG_USER("Fetching headers... done.\r\n");

  // Récupération des levels
  for (k = 0; k < sm->level_nb; k++)
  {
    status = load_level(buf, sm, k);
    if (status == XST_FAILURE)
    {
      return XST_FAILURE;
    }
  }

  free(buf);
  return XST_SUCCESS;
}

/*
int main()
{
  SMfile sm;
  load_sm(&sm, "bag.sm");
  DEBUG_USER("Test fini.\r\n");
  while (1);
}
*/
