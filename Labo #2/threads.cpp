/* NE PAS TOUCHER */
#include <iostream>
#include <vector>
#include <list>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <chrono>
#include <string>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
using namespace std;
#define MAX 7                     // On définie le nombre de thread qu'on veut utiliser
bool done = 0;                    // Variable qui sert à savoir si on a terminé de créer tous les objets
string a[MAX] = {"Free", "Free", "Free", "Free", "Free", "Free", "Free"}; // Tableaux tampons pour recevoir les matériaux pour 
string b[MAX] = {"Free", "Free", "Free", "Free", "Free", "Free", "Free"}; // la fabrication et l'affichage.

/////////////////////////////////////////////////////////////
//AJOUTER VOS INCLUDE ET AUTRES ICI!!!!!!!!!!!!!!!!!!!!!!!!//

#include <mutex>
#include <fstream>
#include <map>



// Mutex pour chaque ressource
mutex ressource_mutex[7];

// Mutex pour synchroniser l'accès au fichier
mutex file_mutex;

// Mon fichier de log 
ofstream log_file("sortieCreation.txt", ios::out);

// Mutex pour gerer l'affichage des messages
mutex message_mutex;

// mutex pour gerer le temps de sortie
mutex temps_mutex;

/////////////////////////////////////////////////////////////



/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! NE PAS TOUCHER !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/**/typedef struct {                                                                                                                           /**/
/**/    string name[MAX]{"Argent", "Or", "Bois", "Metal", "Diamant", "Fer", "Jade"};        // Nom des matériaux                              /**/
/**/    list<string> liste{"Argent", "Or", "Bois", "Metal", "Diamant", "Fer", "Jade"};      // Nom des matériaux dans une liste               /**/
/**/    string craft_name[MAX]{"Coupe", "Porte", "Table", "Bague", "Chandelier", "Epee", "Bracelet"};   // Nom des objets fabriqués            /**/
/**/    int craft[MAX]{0,0,0,0,0,0,0};                                                                                                       // /**/
/**/} craft_t;                                                                                                                                 /**/
/**/                                                                                                                                           /**/
/**/Display *dis;                                                           // Sert pour l'affichage du GUI                                    /**/
/**/int screen;                                                             // Sert pour l'affichage du GUI                                    /**/
/**/Window win;                                                             // Sert à créer la fenêtre pour le GUI                             /**/
/**/GC gc;                                                                  // graphic context                                                 /**/
/**/unsigned long black, white, red, blue, green;                           // Sert pour les couleurs dans le GUI                              /**/
/**/int x, y;                                                               // Sert pour les coordonnées                                       /**/
/**/                                                                                                                                           /**/
/**/void init();                                                            // Fonction pour initialiser le GUI                                /**/
/**/void close();                                                           // Fonction pour fermer le GUI et le programme                     /**/
/**/void refresh();                                                         // Fonction pour rafraîchir l'affichage graphique                  /**/
/**/void menu();                                                            // Fonction pour afficher le menu dans le GUI                      /**/
/**/void afficher(craft_t *c);                                              // Fonction pour afficher les ressources et les objets fabriqués   /**/
/**/unsigned long RGB(int r, int g, int b);                                 // Fonction pour convertir les nombres en couleurs RGB             /**/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/**
 * Fonction : create(int i, craft_t* c)
 * ------------------------------------
 * Cette fonction représente le travail de chaque thread.
 * Chaque thread correspond à un type d’objet à fabriquer (Coupe, Porte, etc.).
 * Il tente d’acquérir deux ressources nécessaires à la fabrication de son objet.
 * Les ressources sont partagées et protégées par des mutex.
 */

void create(int i, craft_t* c) {
    if (c->craft[i] >= 10 || done);  // On ne fait rien si tout les objets atteind 10 c’est déjà fini

    static auto start_time = chrono::high_resolution_clock::now(); // Commence à la première exécution
    static mutex time_mutex;  // pour protéger l'affichage final du temps
    
    int m1, m2;

    // Association des ressources selon le type d’objet
    switch (i) {
        case 0: m1 = 0; m2 = 1; break; // Coupe: Argent + Or
        case 1: m1 = 1; m2 = 2; break; // Porte: Or + Bois
        case 2: m1 = 2; m2 = 3; break; // Table: Bois + Metal
        case 3: m1 = 3; m2 = 4; break; // Bague: Metal + Diamant
        case 4: m1 = 4; m2 = 5; break; // Chandelier: Diamant + Fer
        case 5: m1 = 5; m2 = 6; break; // Epee: Fer + Jade
        case 6: m1 = 6; m2 = 0; break; // Bracelet: Jade + Argent
    }

    // Ajout d’un petit délai aléatoire pour briser la famine
    this_thread::sleep_for(chrono::milliseconds(rand() % 100 + 50));

    bool got_m1 = false, got_m2 = false;

    /**
     * Stratégie d’acquisition des verrous :
     * On choisit aléatoirement l’ordre dans lequel on tente de prendre les deux mutex.
     * Cela évite qu’un ordre fixe crée un blocage circulaire entre threads.
     */
    if (rand() % 2 == 0) {
        got_m1 = ressource_mutex[m1].try_lock();
        if (got_m1) {
            got_m2 = ressource_mutex[m2].try_lock();
            if (!got_m2) {
                ressource_mutex[m1].unlock(); // libère si le 2e n’est pas dispo
            }
        }
    } else {
        got_m2 = ressource_mutex[m2].try_lock();
        if (got_m2) {
            got_m1 = ressource_mutex[m1].try_lock();
            if (!got_m1) {
                ressource_mutex[m2].unlock(); // libère si le 1er n’est pas dispo
            }
        }
    }

    // Si les deux ressources ont été acquises
    if (got_m1 && got_m2) {

      // Mise à jour des noms de ressources pour l’interface graphique
        a[i] = c->name[m1];
        b[i] = c->name[m2];

        // Message d’information (affichage + fichier)
        string message = "Thread : " + to_string(i) + " crée un objet : " + "["
                        + c->craft_name[i]+"]" + " avec " + a[i] + " et " + b[i];

        {
            lock_guard<mutex> msg_lock(message_mutex);// Protège l’accès à cout
            cout << message << endl;
        }
        {
            lock_guard<mutex> file_lock(file_mutex);// Protège l’accès à l'ecriture dans le fichier pour eviter des doublons
            log_file << message << endl;
        }

        // Simulation du travail
        this_thread::sleep_for(chrono::milliseconds(200)); 

        //Incrémente le compteur de cet objet
        c->craft[i]++;

        // Libère les ressources pour les autres threads
        ressource_mutex[m1].unlock();
        ressource_mutex[m2].unlock();

    } else {
        // Si une ressource manque
        a[i] = got_m1 ? c->name[m1] : "Waiting";
        b[i] = got_m2 ? c->name[m2] : "Waiting";

        string waiting_msg = "Thread : " + to_string(i) + " attend : ";
        if (!got_m1) waiting_msg += c->name[m1] + " ";
        if (!got_m2) waiting_msg += c->name[m2];

        {
            lock_guard<mutex> msg_lock(message_mutex);
            cout << waiting_msg << endl;
        }
        {
            lock_guard<mutex> file_lock(file_mutex);
            log_file << waiting_msg << endl;
        }

        // Libère toute ressource éventuellement obtenue
        if (got_m1) ressource_mutex[m1].unlock();
        if (got_m2) ressource_mutex[m2].unlock();
        
        // Pause courte avant de réessayer
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    // Vérifie si tous les objets sont terminés
    bool all_done = true;
    for (int j = 0; j < MAX; ++j) {
        if (c->craft[j] < 10) {
            all_done = false;
            break;
        }
    }

    if (all_done) {
        done = true;

        auto end_time = chrono::high_resolution_clock::now();
        chrono::duration<double> duration = end_time - start_time;

        temps_mutex.lock();
        lock_guard<mutex> time_lock(time_mutex);
        cout << "===============================" << endl;
        cout << "Temps total d'exécution : " << duration.count() << " secondes" << endl;
        cout << "===============================" << endl;
        temps_mutex.unlock();

        {
            temps_mutex.unlock();
            lock_guard<mutex> file_lock(file_mutex);
            log_file << "===============================" << endl;
            log_file << "Temps total d'exécution : " << duration.count() << " secondes" << endl;
            log_file << "===============================" << endl;
            temps_mutex.unlock();
        }
    }
}
// *** Fin fonction create() ******************* //


/*!!!!!!!!!! NE PAS TOUCHER !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
void start(craft_t *c){                                                                         // *** Début fonction start() ****************** //
  vector<thread> threads(MAX);                                                                  // On crée un vecteur de threads.
                                                                                                // 
  for (int i = 0; i < MAX; ++i) {                                                               ///////////////////////////////////////////////////
     threads[i] = thread([i, c] { create(i, c);});                                              // La première boucle crée et utilise un thread
  }                                                                                             // et d'appeler la fonction create().                                             
  for (auto& t : threads) {                                                                     // La deuxième boucle permet de joindre les threads
    t.join();                                                                                   // au fur et à mesure.
  }                                                                                             ///////////////////////////////////////////////////
  cout << "_____________________________________________"<< endl;                               // Ligne permettant de séparer dans le CLI.
}                                                                                               // *** Fin fonction start() ******************** //


/*!!!!!!!!!! NE PAS TOUCHER !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
int main() {  
  
  // Vider le fichier au début de l'exécution
  ofstream clear_file("sortieCreation.txt", ios::out);
  clear_file.close();
                                                                                    // *** Début fonction main() ******************* //
  craft_t craft;                                                                                // Permet de créer l'objet craft, qui va stocker nos informations tout au long du programme.    
  x = 8; y = 10;                                                                                // Permet de configurer les coordonnées de bases pour le GUI.
  init();                                                                                       // Initilialise le GUI.
  XEvent event;                                                                                 // Permet de savoir le type d'evenement.
  char text[1];                                                                                 // Permet d'entreposer la valeur du clavier.
                                                                                                //
  while (1){                                                                                    ///////////////////////////////////////////////////
    afficher(&craft);                                                                           // On affiche dans le GUI.
    y = 10;                                                                                     // On remet le y à 10 pour les prochaines affichages.
    XNextEvent(dis, &event);                                                                    // Se prépare à recevoir l'evenement.
    if(event.type==KeyPress && XLookupString(&event.xkey, text, 255, NULL,0)==1){               // Si il y a eu un evenement (en appuyant sur le clavier):
        if(text[0]=='s'){                                                                       // Option "s":
          if (done != 1){                                                                       // Tant que la fabrication n'est pas terminé,
            start(&craft);                                                                      // On commence un cycle de fabrication d'objet.
          }                                                                                     //
        }                                                                                       // _____________________________________________ //
        if(text[0]=='r'){                                                                       // Option "r":
          if (done != 1){                                                                       // Si la frabrication d'objet n'est pas terminé,
            afficher(&craft);                                                                   // On "rafraîchit" l'affichage.
          }                                                                                     //
        }                                                                                       // _____________________________________________ //
        if(text[0]=='t'){                                                                       // Option "t":
          if (done != 1){                                                                       // Si la frabrication d'objet n'est pas terminé,
            for (int i = 0; i < MAX; ++i) {                                                     // On "reset" la file en remettant chaque ressources,
              a[i] = b[i] = "Free";                                                             // dans un cas où c'est buggé.
              craft.liste ={"Argent", "Or", "Bois", "Metal", "Diamant", "Fer", "Jade"};          //
            }                                                                                   //
          }                                                                                     //
        }                                                                                       // _____________________________________________ //
        if(text[0]=='q'){                                                                       // Option "q":
          close();                                                                              // On ferme le programme.
        }                                                                                       //
    }                                                                                           //
  }                                                                                             ///////////////////////////////////////////////////
                                                                                                //
  return 0;                                                                                     //
}                                                                                               // *** Fin fonction main() ********************* //

/*!!!!!!!!!! NE PAS TOUCHER !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/**/void init(){                                                                                /**/
/**/    x = 100; y = 100;                                                                       /**/
/**/    dis = XOpenDisplay((char *)0);                                                          /**/
/**/    screen = DefaultScreen(dis);                                                            /**/
/**/    black = BlackPixel(dis, screen);                                                        /**/
/**/    white = WhitePixel(dis, screen);                                                        /**/
/**/    red = RGB(255,0,0);                                                                     /**/
/**/    green = RGB(0,255,0);                                                                   /**/
/**/    blue = RGB(0,0,255);                                                                    /**/
/**/    win = XCreateSimpleWindow(dis, DefaultRootWindow(dis),0,0,350,350,5, black, white);     /**/
/**/    XSetStandardProperties(dis,win,"Lab 2 - 8INF342",NULL, None, NULL, 0, NULL);            /**/
/**/    XSelectInput(dis, win, ExposureMask | ButtonPressMask | KeyPressMask);                  /**/
/**/    gc = XCreateGC(dis, win, 0, 0);                                                         /**/
/**/    XSetBackground(dis,gc,white);                                                           /**/
/**/    XSetForeground(dis,gc,black);                                                           /**/
/**/    XClearWindow(dis,win);                                                                  /**/
/**/    XMapRaised(dis, win);                                                                   /**/
/**/}                                                                                           /**/
/**/                                                                                            /**/
/**/void close(){                                                                               /**/
/**/    XFreeGC(dis, gc);                                                                       /**/
/**/    XDestroyWindow(dis,win);                                                                /**/
/**/    XCloseDisplay(dis);                                                                     /**/
/**/    exit(0);                                                                                /**/
/**/}                                                                                           /**/
/**/                                                                                            /**/
/**/void refresh(){                                                                             /**/
/**/    XClearWindow(dis, win);                                                                 /**/
/**/    x = 8;                                                                                  /**/
/**/    y = 10;                                                                                 /**/
/**/}                                                                                           /**/
/**/                                                                                            /**/
/**/void menu(){                                                                                /**/
/**/    x = 8;                                                                                  /**/
/**/    y = 10;                                                                                 /**/
/**/    XSetForeground(dis,gc,BlackPixel(dis, screen));                                         /**/
/**/    XDrawString(dis,win,gc,x,y+200,"Appuyer sur 's' pour cycle",26);                        /**/
/**/    XDrawString(dis,win,gc,x,y+210,"Appuyer 'r' pour refresh page",25);                     /**/
/**/    XDrawString(dis,win,gc,x,y+220,"Appuyer 't' pour reset statut",29);                     /**/
/**/    XDrawString(dis,win,gc,x,y+230,"Appuyer 'q' pour quit",21);                             /**/
/**/    XDrawString(dis,win,gc,x,y+250,"Recipes:",8);                                           /**/
/**/    XDrawString(dis,win,gc,x,y+260,"Argent + Or = Coupe",19);                               /**/
/**/    XDrawString(dis,win,gc,x,y+270,"Or + Bois = Porte",17);                                 /**/
/**/    XDrawString(dis,win,gc,x,y+280,"Bois + Metal = Table",20);                              /**/
/**/    XDrawString(dis,win,gc,x,y+290,"Metal + Diamant = Bague",23);                           /**/
/**/    XDrawString(dis,win,gc,x,y+300,"Diamant + Fer = Chandelier",26);                        /**/
/**/    XDrawString(dis,win,gc,x,y+310,"Fer + Jade = Epee",17);                                 /**/
/**/    XDrawString(dis,win,gc,x,y+320,"Jade + Argent = Bracelet",24);                          /**/
/**/}                                                                                           /**/
/**/                                                                                            /**/
/**/void afficher(craft_t *c){                                                                  /**/
/**/  y = 10;                                                                                   /**/
/**/  XClearWindow(dis, win);                                                                   /**/
/**/  for(int k = 0; k <7; k++){                                                                /**/
/**/    string nbr = to_string(c->craft[k]);                                                    /**/
/**/    XSetForeground(dis,gc,BlackPixel(dis, screen));                                         /**/
/**/    XDrawString(dis,win,gc,x,y,c->craft_name[k].c_str(),c->craft_name[k].size());           /**/
/**/    XDrawString(dis,win,gc,x+42,y," : ",3);                                                 /**/
/**/    XSetForeground(dis,gc,blue);                                                            /**/
/**/    XDrawString(dis,win,gc,x+60,y,nbr.c_str(),nbr.size());                                  /**/
/**/    y = y + 10;                                                                             /**/
/**/  }                                                                                         /**/
/**/  y = y +30;                                                                                /**/
/**/  for(int k = 0; k <7; k++){                                                                /**/
/**/    string kk = to_string(k+1);                                                             /**/
/**/    XSetForeground(dis,gc,BlackPixel(dis, screen));                                         /**/
/**/    XDrawString(dis,win,gc,x,y,"Objet ",6);                                                 /**/
/**/    XDrawString(dis,win,gc,x+72,y,kk.c_str(),1);                                            /**/
/**/    XDrawString(dis,win,gc,x+76,y," : ",3);                                                 /**/
/**/    XSetForeground(dis,gc,red);                                                             /**/
/**/    XDrawString(dis,win,gc,x+102,y,a[k].c_str(),a[k].size());a[k] = "Free";                 /**/
/**/    XDrawString(dis,win,gc,x+152,y,b[k].c_str(),b[k].size());b[k] = "Free";                 /**/
/**/    y = y + 10;                                                                             /**/
/**/  }                                                                                         /**/
/**/  if (done == 1){                                                                           /**/
/**/    XSetForeground(dis,gc,green);                                                           /**/
/**/    XDrawString(dis,win,gc,x,y+10,"Items completes!",15);                                   /**/
/**/  }                                                                                         /**/
/**/  menu();                                                                                   /**/
/**/}                                                                                           /**/
/**/                                                                                            /**/
/**/unsigned long RGB(int r, int g, int b){                                                     /**/
/**/    return b + (g<<8) + (r<<16);                                                            /**/
/**/}                                                                                           /**/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/