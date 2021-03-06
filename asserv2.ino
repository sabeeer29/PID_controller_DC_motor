
// Ce code permet d'asservir en position angulaire un moteur à courant continu.

#include <SimpleTimer.h>

SimpleTimer timer;                 // Timer pour échantillonnage

//definition des entrées
int pinInput1 = 6; // Commande de sens moteur, Input 1
int pinInput2 = 7; // Commande de sens moteur, Input 2
int pinPower = 9;  // Commande de vitesse moteur, Output Enabled1
int encoderPinA = 2;//compteur 1
int encoderPinB = 3;//compteur 2
//int LED=12;
//init echantillonage
unsigned int time = 0;
const int frequence_echantillonnage = 20;

//init compteur :
int encoder0Pos = 0; //position de départ=0
int lastReportedPos = 0; 
boolean A_set = false;
boolean B_set = false;

//consigne
double target_cm = 48;
//double target_tour = 5.23*target_cm;
double target_deg = 68.8*target_cm; //68,83
int target_ticks; //plus simple d'asservir en ticks car ce sera toujours un nombre entier

// init calculs asservissement PID
int erreur = 0; //erreur
float erreurPrecedente = 0;
float somme_erreur = 0;

//Definition des constantes du correcteur PID
float kp = 0.90;               // Coefficient proportionnel    choisis par tatonnement sur le moniteur. Ce sont les valeurs qui donnaient les meilleures performances
float ki = 0; //5.5;          // Coefficient intégrateur
float kd = 0;//100;           // Coefficient dérivateur


/* Routine d'initialisation */
void setup() {

  target_ticks = target_deg/360.0*19.0*64.0;

 
  Serial.begin(115200);         // Initialisation port COM
//pinMode(LED, OUTPUT);
  pinMode(pinPower, OUTPUT);   // Sorties commande  moteur
  pinMode( pinInput1, OUTPUT );
  pinMode( pinInput2, OUTPUT );

  pinMode(encoderPinA, INPUT);  //sorties encodeur
  pinMode(encoderPinB, INPUT); 
  digitalWrite(encoderPinA, HIGH);  // Resistance interne arduino ON
  digitalWrite(encoderPinB, HIGH);  // Resistance interne arduino ON

  // Interruption de l'encodeur A en sortie 0 (pin 2)
  attachInterrupt(0, doEncoderA, CHANGE);
  // Interruption de l'encodeur A en sortie 1 (pin 3)
  attachInterrupt(1, doEncoderB, CHANGE);

  analogWrite(pinPower, 0);  // Initialisation sortie moteur à 0 
  delay(300);                // Pause de 0,3 sec pour laisser le temps au moteur de s'arréter si celui-ci est en marche

  timer.setInterval(1000/frequence_echantillonnage, asservissement);  // Echantillonage pour calcul du PID et asservissement; toutes les 100ms, on recommence la routine
}


void loop(){

  timer.run();  //on fait tourner l'horloge
  //digitalWrite(LED,HIGH); // met la broche au niveau haut (5V) – allume la LED

//delay(500); // pause de 500 millisecondes (ms)

//digitalWrite(LED,LOW); // met la broche au niveau bas (0V) – éteint la LED

//delay(500); // pause de 500ms
 

}

// Cette fonction est appelée toutes les 20ms pour calcul du correcteur PID
void asservissement()
{
  time += 20; // pratique pour graphes excel après affichage sur le moniteur

  erreur = target_ticks - encoder0Pos;
  somme_erreur += erreur;

  // Calcul de la vitesse courante du moteur
  int vitMoteur = kp * erreur + kd * (erreur - erreurPrecedente) + ki * (somme_erreur);

  erreurPrecedente = erreur; // Ecrase l'erreur précedente par la nouvelle erreur

    // Normalisation et contrôle du moteur
  if(vitMoteur > 255) vitMoteur = 255;  // sachant que l'on est branché sur un pont en H L293D
  else if (vitMoteur < -255) vitMoteur = -255;
  Tourner (vitMoteur);

  float angle_deg = encoder0Pos/19.0/64.0*360.0; //Position angulaire de sortie, pratique pour comparer avec la consigne d'entrée
float distance = encoder0Pos/19.0/64.0*360.0/68.83;

  Serial.print(erreur);  // affiche sur le moniteur les données voulues
  Serial.print(" ");
  Serial.print(encoder0Pos);
  Serial.print(" ");
  Serial.print(angle_deg);
  Serial.print(" ");
  Serial.print(distance);
  Serial.print(" ");
  Serial.println(vitMoteur);
}

// Interruption appelée à tous les changements d'état de A
void doEncoderA(){
  A_set = digitalRead(encoderPinA) == HIGH;

  encoder0Pos += (A_set != B_set) ? -1 : +1; //modifie le compteur selon les deux états des encodeurs
}

// Interruption appelée à tous les changements d'état de B
void doEncoderB(){
  B_set = digitalRead(encoderPinB) == HIGH;

  encoder0Pos += (A_set == B_set) ? -1 : +1; //modifie le compteur selon les deux états des encodeurs
}

//Fonction appelée pour contrôler le moteur
void Tourner( int rapportCyclique ){
  if (rapportCyclique > 0) {
    digitalWrite( pinInput1, LOW );
    digitalWrite( pinInput2, HIGH );
  } 
  else {
    digitalWrite( pinInput1, HIGH );
    digitalWrite( pinInput2, LOW );
    rapportCyclique = -rapportCyclique;
  }
  analogWrite( pinPower, rapportCyclique );
}
