#include <graphics.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
 
 // a trial in relation with documents :
 // from Jeff Conrad in PDF format at http://www.largeformatphotography.info/articles/DoFinDepth.pdf
// & http://www.georgedouvos.com/douvos/OptimumCS-Pro_Optical_Science.html


 
 
 // fonctions graphiques utilisées pour adaptation à CANON Magic Lantern
 // setcolor="fontspec_fg" ; setbkcolor="fontspec_bg" ;  outtextxy="bmp_printf" ; g_palettetype ; line(x0,y0,x1,y1)="draw_line" ; bar = "bmp_draw_rect"; setfillstyle ; 
 //  cleardevice="clrscr" ; getpalette ; setrgbpalette
 // in ML : bmp. h : draw_line (x0,y0,x1,y1,color) ; bmp_printf( FONT(FONT_MED, COLOR_WHITE, 0), x, y, " ExpSim " );fontspec_fg;
 // bmp_fill (color, x,y,w,h) ; bmp_draw_rect ; definition de COLOR_WHITE, etc. ; 
 // clrscr
 // bfnt_draw_char(int c, int px, int py, int fg, int bg);
 // 
 //Key :
 // module.h  MODULE_KEY_PRESS_LEFT, (RIGHT), MODULE_KEY_WHEEL_UP, DOWN, RIGHT, LEFT ??
 // ou focus.c 
 // mais aussi quand c'est particulariser pour 700D
        

// to draw from left to right in rule 1/N : First-init-stick is at 1/.01 then 100 meter or feet ; N = 0 is infinity ; N=0.5 is 2 unit ;
// N gives the granurality of each step modulo the accuracy
static  double N_first_right=0.0, N_last_left=10.0, Nb_N_in_slot = 0.01; // the represented value is in 1/ N. .52
static  double N_Accuracy = 1.0;
static  double N_Granularity = 4.0; // initially to 1.0
static  double Min_dist = 10.0; // can not go down to 0.1m ex. : 10 cm of focus distance / hyperfocale
static  double Percentage_Diffraction=0.0;


static unsigned int F_min = 15 , F_max = 80, Current_F = 55; // the focal in mm - can be a lens with zoom // inherit
//static int N_magnifier_Nb_Pixel_in_slot = 1;// in coefficient --> x0.1 means 1 stick for 1 meter indicates 1 stick for 10 cm 
static unsigned long Current_dist = 150; // cm
static double N_H;

static double C_def;
static double C_diffr;
static double C_T;

//diffraction const
// marque (model) -captor type-size in mm - crop factor - coc=D/1730(limit of human eyes-coc=D/1440 more permissive and used everywhere  where D is the diagonal or sqrt (h2+w2)
//Canon 	(40D, 350D, 400D, 450D, 1000D, 1100D) 	aps-c 	14,8 x 22,2	1,62	0,015	0,019
//Canon 	7D, 50D, 60D, 60Da, 500D, 550D, 600D,650D 	aps-c 	14,9 x 22,3	1,61	0,016	0,019
//Canon 	20D, 20Da, 30D 	aps-c 	15 x 22,5	1,60	0,016	0,019
//Canon 	10D, 300D, D60, D30 	aps-c 	15,1 x 22,7	1,59	0,016	0,019
//Canon 	1D-Mark-IV 	aps-h 	18,6 x 27,9	1,29	0,019	0,023
//Canon 	1D Mark III 	aps-h 	18,7 x 28,7	1,26	0,020	0,024
//Canon 	1D Mark II, 1D Mark II N 	aps-h 	19,1 x 28,7	1,26	0,020	0,024
//Canon 	EOS-1Ds 	full frame 	23,8 x 35,8	1,01	0,025	0,030
//Canon 	EOS-6D 	full frame 	23,9 x 35,8	1,01	0,025	0,030
//Canon 	1Ds-Mark-II, 1Ds-Mark-III, 5D-Mark-II,1D-X, 5D-Mark-III, 1D-C 	full frame 	24 x 36	1,00	0,025	0,030


static int lgonde = 560; // in nm normally to divide by 1000 0,56 microns is the longueur d'onde of yellow-green light which is the best for human eyes
static  int diag = 1440; // std by default - to be less severe = 1440 (use for 0.019 APS-C or 0.030 full), stricter 3000, std = 1730 (0.015 or 0.025)

// See Jeff Conrad PDF at http://www.largeformatphotography.info/articles/DoFinDepth.pdf
// & http://www.georgedouvos.com/douvos/OptimumCS-Pro_Optical_Science.html
static double cocx = 0.019;
static double coc;
static unsigned int calc_w_diff = 1; // Boolean init to true
static unsigned int Flag_too_much_diffrac =0;
static double N_Min, N_Max, Klambda, Delta_v;  
static double RP; // idem Resolving Power
static double magnify ; // page 6 conrad : c'est le rapport entre u, distance entre l'objet et la lentille  et v distance entre la lentille et le capteur ; m=v/u traduit en u-f=f/m => m=f/u-f


static int unit_measure = 1; // meter else in feet (reprendre la variable de magic_lantern) // to inherit  and adapt

static  double Focus_distance = 10.0; // the current position of the focus distance in meter or feet
static  double Magnifier = 1.0; // to magnify or not a slot size in # pixels

//assume than the (0,0) is upper/left // to check 
// in pixels
// GM_1024x768
// GM_800x600
//GM_640x480
static const int  Screen_width = 1000; //720;
static const int Rule_Width = 980; // 700 to inherit (max-20)
static const int Max_y_ecran = 720; // 480; // to inherit
static const int X_Rule_Position = 10;
static int last_rule_y_position = 190; // position of the upper line
static const int Rule_height = 50; // position of the lower line
static int CurrentPixelPosition = 980; // 700;
static int NextPixelPositionL1 = 0;
static int NextPixelPositionL2 = 0;
static int Offset = 0;

static const int Between_Tick_step = 5; // 5 pixels a minima between two ticks to display
static int Large_w_tick = 12;
static int Medium_w_tick = 8;
static int Small_w_tick = 4;
static int pos_text = 30;
static int pos_text2 = 25;
static int pos_text3 = 20;

// different cycle 
static double cycle1 = 100.0;
static double cycle2 = 10.0;
static double cycle3 = 1.0;
static int count = 0;

static double nbr_slot_intertick ;//=  Rule_Width/Between_Tick_step; // 140.0; // slot by accuracy (init 1/100 or 0.01) a slot is more or less a space between tick
static double Nb_Pixel_in_slot ;//= Between_Tick_step; //Rule_Width/nbr_slot_intertick ;// pixel size of slot -- un slot tous les Nb_Pixel_in_slot pixels (700 / 50) envir 14 pixels


// to inherit/adapt in the code
static const double values_aperture_tiers[] = {1.2,1.6,1.8,2.2,2.5,3.2,3.5,4.0,4.5,5.0,6.3,7.1,9.0,10.0,13.0,14.0,18.0,20.0,25.0,29.0,36.0,40.0}; 
static const double values_aperture_demi[] = {1.3,2.5, 3.5, 4.5, 6.7,9.5,13.0,19.0,27.0,38.0};
static const double values_aperture_full[] = {1.4,2.0,2.8,4.0,5.6,8.0,11.0,16.0,22.0,32.0, 45.0}; 

static const int focals[] = {15,24,35,50,70,100,135,200,250,300,400,500,600,800}; // to know the step to display - 14, 15, 16, 17, 18 ? 20 ? 22?
// lens examples : 24, 50, 15-85 (15,24,35,50,70,85), 55-250 (55,70,100, 135,200, 250)
// il existe 10-22, 17-55,18-55, 60, 8-15, 16-35, 17-40, 24-105, 100-400, 14, 500, 600, 800, 185, 65, 45, 90
// the problem is to adapt for lens with variable focal. The one-focal lens is in the current focal, thus no problem for it
static const int nb_focal_line = 12;
static const int nb_apertures = 9;

static double HDs[9][12]; //[nb_apertures][nb_focal_line]; // 
static double Currents_Focal_HD[9]; //by nb_apertures;
static double Currents_Diff[9]; // by nb_apertures;
static int index_lens=0;
struct Focal_Pixel
{
	int length;
	int pixel;
	char text[5];
};
		
void init_Focals_HD ();
   
//   AfterComma : min 1 max 8
void _fl2str (float FlVal, short AfterComma, int flLen, char *data)
{
	int PartieEntiere=(int)FlVal;
	int Factor = 10;	
	
	for (int i =1; i<AfterComma;i++) Factor *= 10;
	int Reste = (int)((float)(FlVal-(float)PartieEntiere)*Factor);
	if (PartieEntiere <0 && Reste != 0) PartieEntiere = -1*PartieEntiere;
	if (Reste < 0) {
		Reste = -1*Reste;
		//snprintf(data, flLen, "-%d.%08d", PartieEntiere, Reste);
		switch (AfterComma) {//
			case 1 : snprintf(data, flLen, "-%d.%01d", PartieEntiere, Reste);break;
			case 2 : snprintf(data, flLen, "-%d.%02d", PartieEntiere, Reste);break;
			case 3 : snprintf(data, flLen, "-%d.%03d", PartieEntiere, Reste);break;
			case 4 : snprintf(data, flLen, "-%d.%04d", PartieEntiere, Reste);break;
			case 5 : snprintf(data, flLen, "-%d.%05d", PartieEntiere, Reste);break;
			case 6 : snprintf(data, flLen, "-%d.%06d", PartieEntiere, Reste);break;
			case 7 : snprintf(data, flLen, "-%d.%07d", PartieEntiere, Reste);break;
			case 8 : snprintf(data, flLen, "-%d.%08d", PartieEntiere, Reste);break;
		default : snprintf(data, flLen, "-%d.%08d", PartieEntiere, Reste);break;
		}; // end switch AfterComma		
	} // pb si reste ==0 et PartieEntiere <0 
	else 
		switch (AfterComma) {//
			case 1 : snprintf(data, flLen, "%d.%01d", PartieEntiere, Reste);break;
			case 2 : snprintf(data, flLen, "%d.%02d", PartieEntiere, Reste);break;
			case 3 : snprintf(data, flLen, "%d.%03d", PartieEntiere, Reste);break;
			case 4 : snprintf(data, flLen, "%d.%04d", PartieEntiere, Reste);break;
			case 5 : snprintf(data, flLen, "%d.%05d", PartieEntiere, Reste);break;
			case 6 : snprintf(data, flLen, "%d.%06d", PartieEntiere, Reste);break;
			case 7 : snprintf(data, flLen, "%d.%07d", PartieEntiere, Reste);break;
			case 8 : snprintf(data, flLen, "%d.%08d", PartieEntiere, Reste);break;
		default : snprintf(data, flLen, "%d.%08d", PartieEntiere, Reste);break;
		}; // end switch AfterComma		
		
		//snprintf(data, flLen, "%d.%08d", PartieEntiere, Reste);
}
			
			
static struct Focal_Pixel focal_line[12]; //[nb_focal_line];
//static double A, B; 
// A est la pente (slote or gradient in english) de la droite et B sa translation (offset) between X and Y axis : straight line to display apertures amongst focal

static double Slote_aperture_line, Offset_aperture_line;

// We can add a magnifier of the focal line and then to display in a windowd some them ; not all 
// We then must add a scroll to go to the F_min and F_max and a zoom in and out
// Is it necessary ? 
double aperture_min=1.4, aperture_max=22.0; // to inherit
double Current_aperture = 4.0;
double Current_aperture_AV = 4.0; // to facilate the passage to next/previous aperture

// extra

    g_palettetype pal;



double the_closest_aperture (double MyAperture)
{
	double aperture_found;

		for (int i = 0; i < sizeof(values_aperture_tiers)/sizeof(values_aperture_tiers[0]); i++)
			if (MyAperture < values_aperture_tiers[i]) {
					aperture_found=values_aperture_tiers[i-1]; // assume that the i is >0
					break;
			} else if (MyAperture == values_aperture_tiers[i])  return values_aperture_tiers[i];
		//printf("	the_closest_aperture : tiers found %f\n", aperture_found);
			  
		for (int i = 0; i < sizeof(values_aperture_demi)/sizeof(values_aperture_demi[0]); i++)
			if (MyAperture < values_aperture_demi[i]) {
				if ((MyAperture-aperture_found) > (MyAperture-values_aperture_demi[i-1])) 
					aperture_found=values_aperture_demi[i-1];
				break;
			} else if (MyAperture == values_aperture_demi[i]) return values_aperture_demi[i];
		//printf("	the_closest_aperture : demi found %f\n", aperture_found);

				  		  	
		for (int i = 0; i < sizeof(values_aperture_full)/sizeof(values_aperture_full[0]); i++)
			if (MyAperture < values_aperture_full[i]) {
				if ((MyAperture-aperture_found) > (MyAperture-values_aperture_full[i-1]))
					return values_aperture_full[i-1];
				else
					return aperture_found;
			} else if (MyAperture == values_aperture_full[i]) return values_aperture_full[i];
}


void mini_triangle(int x, int y)
{
	int my_y = y-6;
	if (my_y < 0) my_y = 0;
	if (x >= 6) {
	line (x-6, my_y,x,y);
	line (x+6, my_y,x,y);
	line (x-6, my_y,x+6,my_y);
	}
}

void Set_cycles (int ValueTick1)
{
	//fprintf(stderr, "Set_cycles valuetick %d--N_Granularity %g et division =%g kaka\n", ValueTick1, N_Granularity, ValueTick1/N_Granularity);
if (ValueTick1/N_Granularity > 100) {cycle1 = 100.0;cycle2=50.0;cycle3=10.0;} else
if (ValueTick1/N_Granularity > 50) {cycle1 = 50.0;cycle2=10.0;cycle3=5.0;} else
if (ValueTick1/N_Granularity > 10) {cycle1 = 10.0;cycle2=5.0;cycle3=1.0;} else
if (ValueTick1/N_Granularity > 5) {cycle1 = 5.0;cycle2=1.0;cycle3=0.5;} else
if (ValueTick1/N_Granularity > 1) {cycle1 = 1.0;cycle2=0.5;cycle3=0.1;} else 
if (ValueTick1/N_Granularity > 0.5) {cycle1 = 0.5;cycle2=0.1;cycle3=0.05;} else 
if (ValueTick1/N_Granularity > 0.1) {cycle1 = 0.05;cycle2=0.01;cycle3=0.01;} else // usefull ?
{cycle1 = 0.1;cycle2=0.05;cycle3=0.01;};	
}



int show_tick_and_value(double DistValue, int SizeTick)
 // SizeTick = 0 for high, 1 for medium, 2 for small
{
	char Dist_str [8];
	int center = 3; // 3 is mid font -- to change
	long Entier, ValueE, ByTen;
	int stop = 0;
	long Current_x_pixel;
	
	setcolor(WHITE);
	setbkcolor(BLACK);
	
if (DistValue < 0.0) return (1);
if (DistValue >= 10000.0) {Current_x_pixel=(Rule_Width + X_Rule_Position + Offset); // infinity case
	} else {
	// I.calculate pixel position 
	Current_x_pixel = (long)(((N_last_left - (1.0/DistValue))/Nb_N_in_slot)*Nb_Pixel_in_slot)+Offset+X_Rule_Position; 

	
	if (abs(CurrentPixelPosition - Current_x_pixel) <= Between_Tick_step) {
			return(1); }; 
			
	if (abs(Current_x_pixel - NextPixelPositionL1) <= Between_Tick_step) {
			return(1);}; 
			
	if (abs(Current_x_pixel - NextPixelPositionL2) <= Between_Tick_step) {
			return(1);}; 
	}; //(DistValue >= 10000.0) 
	if ((Current_x_pixel < X_Rule_Position) || (Current_x_pixel > Rule_Width+X_Rule_Position)) { stop=1;}; 
	if (stop == 0) {
		
	// II.a.display tick lower line
	if (SizeTick == 0) // high
		line (Current_x_pixel,last_rule_y_position+Rule_height,Current_x_pixel,last_rule_y_position+Rule_height-Large_w_tick);
	else if (SizeTick == 1) // medium
		line (Current_x_pixel,last_rule_y_position+Rule_height,Current_x_pixel,last_rule_y_position+Rule_height-Medium_w_tick);
	else // small
		line (Current_x_pixel,last_rule_y_position+Rule_height,Current_x_pixel,last_rule_y_position+Rule_height-Small_w_tick);

	// II.b.display tick upper line
	if (SizeTick == 0) // high
		line (Current_x_pixel,last_rule_y_position,Current_x_pixel,last_rule_y_position+Large_w_tick);
	else if (SizeTick == 1) // medium
		line (Current_x_pixel,last_rule_y_position,Current_x_pixel,last_rule_y_position+Medium_w_tick);
	else // small
		line (Current_x_pixel,last_rule_y_position,Current_x_pixel,last_rule_y_position+Small_w_tick);

		
		// III.display value 
	if (SizeTick < 1)  {
		if (DistValue>=10000.0) {
			snprintf (Dist_str,sizeof(Dist_str),"inf");
		}else {
			snprintf (Dist_str, sizeof(Dist_str), "%.2f", DistValue);
			sscanf (Dist_str, "%d.%1d%1d", &Entier, &ByTen, &ValueE);
		
		if (ValueE==0 && ByTen==0) {
			snprintf (Dist_str, sizeof(Dist_str), "%d", Entier);
		} else {
			if (ValueE==0) {
				snprintf (Dist_str, sizeof(Dist_str), "%.1f", DistValue); //Entier, ValueE);
			} else {
				snprintf (Dist_str, sizeof(Dist_str), "%.2f", DistValue); //Entier, ValueE);
			}
		}
		}; // else 
			
		center = strlen(Dist_str) * 7; // assume width font is 7
		switch (SizeTick) {
			case 1 : Entier=pos_text;break; // + count*5; count=(count+1)%2 ;break;
			case 0 : Entier=pos_text3+count*10; count=(count+1)%3 ;;break;
			case 2 : Entier=pos_text3;break;
			default : Entier=pos_text3;break;
		}
		setcolor(WHITE); // YELLOW);
		setbkcolor(BLACK);
		outtextxy (Current_x_pixel-center/2, last_rule_y_position+Rule_height-Entier,  Dist_str);
	};
	
	if (SizeTick==0) {NextPixelPositionL1=Current_x_pixel;}; 
	if (SizeTick==1) {NextPixelPositionL2 = Current_x_pixel;}; 
	if (SizeTick==2) {CurrentPixelPosition = Current_x_pixel;}; 
	}; // stop == 0

	
	return(1);
}


void refresh_coc ()
{
		C_diffr = (Current_aperture*(1+magnify)/Klambda);
		if (calc_w_diff) {
				  if ((cocx*cocx) - (C_diffr*C_diffr) < 0.0) 
				  	{fprintf(stderr, "ERROR coc negatif ; diffraction blur is too much \n"); 
				  		coc=cocx;
				  		Flag_too_much_diffrac = 1;
				  		//calc_w_diff = 0;
				  	}else { 
						  coc=sqrt((cocx*cocx) - (C_diffr*C_diffr));
						  Flag_too_much_diffrac = 0;
						  
					};
		}else{
					coc=cocx;
					 Flag_too_much_diffrac = 1;
		};
		init_Focals_HD ();
}


// if movable == 0 then it is not possible to move all the rule
void draw_dof_rule () // imagine to display from min to max and N_step is between left-right
{
double Dist_from_N, M, N, DistL2;
char inf_str[7];
char txt_str[25];
int center = 20;

CurrentPixelPosition=Rule_Width+X_Rule_Position; // Between_Tick_step
NextPixelPositionL1=0;
NextPixelPositionL2=0;

// Initialize  Bar of balance between diffraction and defocus : aim to have 50/50 (see Conrad DoFinDepth p 28 (after equation 114)
	setcolor(YELLOW);
	setfillstyle(SOLID_FILL, CYAN);
  bar (Screen_width/2,Max_y_ecran+10,Screen_width/2+100,Max_y_ecran+20); //defocus bar
	setfillstyle(SOLID_FILL, BLACK);
	snprintf (txt_str, sizeof(txt_str), "diffraction");
	setbkcolor(BLACK);
	setcolor(LIGHTGRAY);
  outtextxy(Screen_width/2-100, Max_y_ecran,txt_str);

	snprintf (txt_str, sizeof(txt_str), "defocus");
	setbkcolor(BLACK);
	setcolor(CYAN);
  outtextxy(Screen_width/2+100, Max_y_ecran,txt_str);

	snprintf (txt_str, sizeof(txt_str), "nice=50/50");
	setbkcolor(BLACK);
	setcolor(GREEN);
  outtextxy(Screen_width/2+10, Max_y_ecran,txt_str);
  line(Screen_width/2+50,Max_y_ecran+10,Screen_width/2+50,Max_y_ecran+20);
		
	setcolor(BLACK);
	setbkcolor(BLACK);
	bar (0,last_rule_y_position,Screen_width,last_rule_y_position+Rule_height); //raz the background of the rule
	setcolor(WHITE);
	setbkcolor(BLACK);
	line(X_Rule_Position,last_rule_y_position,Rule_Width+X_Rule_Position, last_rule_y_position);
	line(X_Rule_Position,last_rule_y_position+Rule_height,Rule_Width+X_Rule_Position, last_rule_y_position+Rule_height);
	setcolor(BLACK);

if (N_first_right == 0.0) { // infinity case
    Dist_from_N =  200.0;
    //printf("draw_dof_rule then call set_cycles\n");
    Set_cycles(Dist_from_N);
    } else { // other than infinity
    Dist_from_N=(1.0/N_first_right);    
       // printf("draw_dof_rule else appel set_cycles\n");
    Set_cycles(Dist_from_N);
    Dist_from_N= (double)(((long)(Dist_from_N/cycle1))*cycle1)+cycle1;
    }; //endif else (N_first_right==0)

// Set cycles regarding the entry value - here Before_comma
// Set_cycles(Dist_from_N);

// init first tick value
    if (Dist_from_N<=(1.0/N_last_left))
    {    Dist_from_N=(1.0/(N_last_left));
    };

//display first tick cycle1

if ((N_first_right != 0.0) && (Dist_from_N < (1.0/N_first_right))) {
    if (show_tick_and_value(Dist_from_N, 0)==0) return;
    NextPixelPositionL2=0;
    CurrentPixelPosition=NextPixelPositionL1;
    NextPixelPositionL1=0;
    if (CurrentPixelPosition==0) CurrentPixelPosition=Rule_Width+X_Rule_Position;
};

if (N_first_right == 0.0)  {
        if (show_tick_and_value(10000.0, 0)==0) return;
    NextPixelPositionL2=0;
    CurrentPixelPosition=NextPixelPositionL1;
    NextPixelPositionL1=0;
    if (CurrentPixelPosition==0) CurrentPixelPosition=Rule_Width+X_Rule_Position;
};

int stop=1;

for (double DistL1=Dist_from_N; (stop && (DistL1 > (1.0/N_last_left))) ;
DistL1 = M)
{
    //nextL1
    M =(DistL1-cycle1);
    if (M<cycle1) M=cycle1; // reach the lower limit of cycle1

    //test in the lower boundary
    if (M<=(1.0/N_last_left))
    {
        M = 1.0/(N_last_left); //-Nb_N_in_slot); 
        stop = 0;
    };
    if (show_tick_and_value(M, 0)==0) return;
    // NextPixelPositionL1 is set

    // tick cycle2
    NextPixelPositionL2=0;

    for (double DistL2=DistL1; ((DistL2 >M)&&(DistL2>(1.0/N_last_left))) ; DistL2 = N)
    {
        N= DistL2-cycle2;
        //printf ("\tL2=[%.2f,%.2f]--\n",DistL2, N);
        if ((N > M) && (N > (1.0/N_last_left)) && (N <(1.0/N_first_right))) {
            if (show_tick_and_value(N, 1)==0) return;
        // NextPixelPositionL2 is also set
        }else {
            N=M; // reach the cycle1 boundary
            NextPixelPositionL2=NextPixelPositionL1;
        }
        // tick cycle3
        for (double DistL3=(DistL2-cycle3); ((DistL3 > N) && (DistL3 >M)); DistL3-=(cycle3))
        {
            if (DistL3 < (1.0/N_first_right)) 
				if (show_tick_and_value(DistL3, 2)==0) return;
        } // for
        //printf("\n");
        CurrentPixelPosition=NextPixelPositionL2;
        NextPixelPositionL2=0;
    } // for 2

    // next cycle

CurrentPixelPosition=NextPixelPositionL1;
if (CurrentPixelPosition==0)
CurrentPixelPosition=Rule_Width+X_Rule_Position;
NextPixelPositionL1=0;
NextPixelPositionL2=0;

// reinit cycles according to the new value M or cycle1

Set_cycles (M);

} //for  1

//Current distance marker
	long Current_x_pixel = ((long)(((N_last_left - N_H)/Nb_N_in_slot)*Nb_Pixel_in_slot))+Offset+X_Rule_Position;


	int topbar = last_rule_y_position-15;
	// effacement
	
	if (topbar < 0) topbar = 0;

	bar(Current_x_pixel-20, topbar,Current_x_pixel+40,last_rule_y_position-1); // raz current position text

	setcolor(WHITE);
	setbkcolor(BLACK);
	mini_triangle(Current_x_pixel, last_rule_y_position);
	

	
	if (Current_dist >= 10000000)
		snprintf (inf_str, sizeof(inf_str), "inf");
	else
		snprintf (inf_str, sizeof(inf_str), "%f", 1.0/N_H); // affiche la distance courante
	if (Current_x_pixel-10 >0){
			//setcolor(WHITE);
			setcolor(RED);setbkcolor(BLACK);
			outtextxy(Current_x_pixel-10, last_rule_y_position-17,inf_str);
	};

}// end draw_dof_rule

double focus_calc_focal_N_H(int focal, double aperture, double LocalCoc)
{
     // Calculate hyperfocal distance H round to F
    long H = ((long)(((focal*focal)/(aperture * LocalCoc))+focal));
    return (1000.0/H);
}

long focus_calc_focal_H(int focal, double aperture, double LocalCoc) // in mm
{
     // Calculate hyperfocal distance H round to F
    return ((long)(((focal*focal)/(aperture * LocalCoc))+focal));
    
}
// init the min and max values for the rules according with the current focal and display rule dof at 22 aperture
// then init the rules 
void init_rule_dof()
{

  Klambda = 1000000.0/(2.44*lgonde);
 	
	long Current_dist_mm = (long) Current_dist*10.0; // ((focus_calc_focal_H (Current_F, 25.0, coc))/10)*10;
	//fprintf(stderr, "Init Focus Dist=%d\n", Current_dist_mm);
	//printf("Klambda=%f||", Klambda);
	nbr_slot_intertick =  Rule_Width/Between_Tick_step; // 140.0
	Nb_Pixel_in_slot = Between_Tick_step; // init 5 pixels
	
	N_H = (1000.0/Current_dist_mm); // in meter
	
  double fd = Current_dist_mm/1000.0; // Current_dist / 100.0; // into meter
	double N_fd = N_H; //Current_dist;
	int bottom_offset =  Rule_height;
	int	top_offset = 15;
	
	if (F_max>F_min)  {// not mono focal else take the middle 
		Slote_aperture_line = ((double)(Max_y_ecran-bottom_offset-top_offset))/((1/(double)(F_max*F_max))-(1/(double)(F_min*F_min)));
		Offset_aperture_line = ((double)(Max_y_ecran-bottom_offset)) - (Slote_aperture_line*(1/(double)(F_max*F_max)));
		last_rule_y_position = Slote_aperture_line*(1/(double)(Current_F*Current_F)) + Offset_aperture_line;
	} else {
		Slote_aperture_line = 0;
		Offset_aperture_line = last_rule_y_position;
	};	
	N_first_right =  0.0; //N_fd - N_H; // to check is not negatif ?? XXX
	N_last_left = N_H + N_H; // this is a choice N_fd+N_H;
	Offset = 0; 
	Nb_N_in_slot=(N_last_left-N_first_right)/nbr_slot_intertick; // 140 slots
	
	magnify = Current_F/(Current_dist_mm-Current_F);//all units in mm
	printf("init magnify=%f--", magnify);
	
} // init rule dof

void draw_linear_rule ()
{
	
setcolor(WHITE);setbkcolor(BLACK);
for (int i = 0; i<=nbr_slot_intertick;i++) 
	if ((i%10)==0){
		line (X_Rule_Position+i*Nb_Pixel_in_slot,last_rule_y_position+Rule_height+1, X_Rule_Position+i*Nb_Pixel_in_slot,last_rule_y_position+Rule_height+10);
	} else if ((i%5)==0){
		line (X_Rule_Position+i*Nb_Pixel_in_slot,last_rule_y_position+Rule_height+1, X_Rule_Position+i*Nb_Pixel_in_slot,last_rule_y_position+Rule_height+5);
	}else{
		line (X_Rule_Position+i*Nb_Pixel_in_slot,last_rule_y_position+Rule_height+1, X_Rule_Position+i*Nb_Pixel_in_slot,last_rule_y_position+Rule_height+2);
	};
}

void show_bar_cocs ()
{
		char inf_str[25];
		
  //Percentage diffraction/defocus	changer le coc réelle et l'afficher
  Percentage_Diffraction = 100.0* (C_diffr*C_diffr)/(C_T*C_T);
  //printf("percentage diffraction : %d\n", (int)Percentage_Diffraction);
	setfillstyle(SOLID_FILL, BLACK);
	snprintf (inf_str, sizeof(inf_str), "coc= %0.4f %.0f %%", C_diffr, Percentage_Diffraction); 
	setbkcolor(BLACK);
	setfillstyle(SOLID_FILL, BLACK);
  bar (Screen_width/2-140,Max_y_ecran+10,Screen_width/2-60,Max_y_ecran+40);

	if  (Flag_too_much_diffrac && calc_w_diff) {
	setcolor(RED);
}else{
	setcolor (LIGHTGRAY);
}
  outtextxy(Screen_width/2-120, Max_y_ecran+10,inf_str);
	if  (Flag_too_much_diffrac && calc_w_diff) {
  	 snprintf (inf_str, sizeof(inf_str), "too much");
  	 outtextxy(Screen_width/2-120, Max_y_ecran+20,inf_str);
  	};
  
  if (Percentage_Diffraction > 100.0) Percentage_Diffraction=100.0;
  setfillstyle(SOLID_FILL, LIGHTGRAY);
  bar (Screen_width/2,Max_y_ecran+10,Screen_width/2+(int)Percentage_Diffraction,Max_y_ecran+20);
	setfillstyle(SOLID_FILL, BLACK);


  
  Percentage_Diffraction = ((100.0* C_def*C_def)/ (C_T*C_T));
	snprintf (inf_str, sizeof(inf_str), "%.0f %% coc=%0.4f", Percentage_Diffraction,C_def); //ZZZ
	setbkcolor(BLACK);
	
		setcolor(YELLOW);

  outtextxy(Screen_width/2+110, Max_y_ecran+10,inf_str);

	setcolor(GREEN);
  line(Screen_width/2+50,Max_y_ecran+10,Screen_width/2+50,Max_y_ecran+20);

	setfillstyle(SOLID_FILL, BLACK);
	bar (Screen_width/2-35,Max_y_ecran+24,Screen_width/2+200,Max_y_ecran+35);
	
	if ((int)(10000*C_T)>(int)(10000*(cocx+0.00002))) {
	snprintf (inf_str, sizeof(inf_str), "coc total= %0.4f", C_T); 
	setbkcolor(BLACK);
	setcolor(YELLOW);
  outtextxy(Screen_width/2-35, Max_y_ecran+27,inf_str);
	snprintf (inf_str, sizeof(inf_str), "%0.4f", cocx); 
	setbkcolor(BLACK);
	setcolor(RED);
  outtextxy(Screen_width/2+90, Max_y_ecran+27,inf_str);
} else {
	snprintf (inf_str, sizeof(inf_str), "coc total= %0.4f", C_T); 
	setbkcolor(BLACK);
	setcolor(YELLOW);
  outtextxy(Screen_width/2-35, Max_y_ecran+27,inf_str);
};
}

void draw_current_aperture () 
{
		long Current_x2_pix=0;
	char inf_str[25];

	
	//Current_aperture
	long ZeY= (int)(Slote_aperture_line*(1.0/(double)(Current_F*Current_F)) + Offset_aperture_line);
	double Dnf=(double)Current_dist *(focus_calc_focal_H(Current_F, Current_aperture, coc)-Current_F)*10.0; // in mm // base for the string to display
	double Dn, Df, Dnf2, Dn1, Df1;
	double NearDist, FarDist;
	double Ze_Current_Diff_blur, Defocus_Blur_Aim;
	
	long Current_x_pixel, Current_x_pixel_near, Current_x_pixel_near_2, Current_x_pixel_far, Current_x_pixel_far_2;
	
	if ((1/(N_H-focus_calc_focal_N_H(Current_F, Current_aperture, coc))-1/N_H) >= 0.0) //on suppose N_H le 1/current_distance  dec 2017 donc ici (1/(1/s-1/H)-s) = Df-s
			//Df=s(H-f)/(H-s) ou Dnf/(H-s)
			Df = Dnf/(focus_calc_focal_H(Current_F, Current_aperture, coc)-(Current_dist*10.0)); // in mm
	else 
			Df = 10000000.0; // infinity
	
	Df1=Df;
	Current_x_pixel = (long)  (((N_last_left-(1000.0/Df)) /Nb_N_in_slot)*Nb_Pixel_in_slot)+Offset+X_Rule_Position; 
	
	 Ze_Current_Diff_blur = (1+magnify)*(Current_aperture*2.44*(lgonde/1000000.0));// /diag; // diffraction blur via some approximation in micron-meter
	
	// coc is the aim of total blur /// then we calculate the distances (near and far) where the real blur is lower than declared coc
	// then calculate an objectif of blur spot in defocus to have the near and far limits of total
	Defocus_Blur_Aim = (coc*coc-Ze_Current_Diff_blur*Ze_Current_Diff_blur); // Focus & diffraction blur by approximation 
	// we assume that the current coc is coc, but may be not !!!
	

  
// far distance **********************************************************************************************

	if (last_rule_y_position !=Max_y_ecran/2+4) {
	setcolor(BLACK);
	
	line (X_Rule_Position, ZeY-6+Rule_height/2,Rule_Width+X_Rule_Position ,ZeY-6+Rule_height/2); // to refresh and delete the line assume that the black is the bg color
	};
	
	setcolor(YELLOW);setbkcolor(BLACK);
	
	if ((Current_x_pixel > 0)&&(Current_x_pixel<Screen_width)) 
		line (Current_x_pixel,ZeY, Current_x_pixel,ZeY+Rule_height); // draw line on the larger on the rule at position of far	
	
	if (Current_x_pixel>=Screen_width) Current_x_pixel = X_Rule_Position+Rule_Width; //+Offset;
		setcolor(YELLOW); 
		line (Current_x_pixel, ZeY-6+Rule_height/2, Screen_width/2, ZeY-6+Rule_height/2);//horizontal line of dof dist inside the rule beginning always (of terminating) at mid-screen
		
		
		if ((1/(N_H-focus_calc_focal_N_H(Current_F, Current_aperture, coc))-1/N_H) >= 0.0){ //on suppose N_H le 1/current_distance  dec 2017 donc ici (1/(1/s-1/H)-s) = Df-s
			//Df=s(H-f)/(H-s) ou Dnf/(H-s)
			setfillstyle(SOLID_FILL, WHITE);
			bar (Screen_width/2 + 10, ZeY-6+Rule_height/2,Screen_width/2 + 55, ZeY+6+Rule_height/2);
			snprintf (inf_str, sizeof(inf_str), "<%.3f>", (Df-(Current_dist*10.0))/1000.0); // 1/(N_H-focus_calc_focal_N_H(Current_F, Current_aperture)) );//-1/N_H);
			setcolor(BLACK); 
			outtextxy (Screen_width/2 + 10, ZeY-6+Rule_height/2, inf_str);
			snprintf (inf_str, sizeof(inf_str), "%.3f", Df/1000.0);
			setfillstyle(SOLID_FILL, BLACK);
			bar (Current_x_pixel-5, ZeY-20,Current_x_pixel + 50, ZeY-5);
			setcolor(YELLOW);setbkcolor(YELLOW);
			outtextxy(Current_x_pixel-5,ZeY-17, inf_str);
			FarDist = Df;
		}else{
			snprintf (inf_str, sizeof(inf_str), "inf");
			FarDist = 100000;
			;
		};
		setfillstyle(SOLID_FILL, WHITE);

		setfillstyle(SOLID_FILL, BLACK);
		setcolor(YELLOW);

				
// diffraction display 		
		Dnf2=0.0;
		if (Defocus_Blur_Aim > 0) 
		Dnf2=(double)Current_dist *(focus_calc_focal_H(Current_F, Current_aperture, sqrt(Defocus_Blur_Aim))-Current_F)*10.0; // in mm // base for the string to display

	if (Defocus_Blur_Aim > 0) {
			if ((1/(N_H-focus_calc_focal_N_H(Current_F, Current_aperture, sqrt(Defocus_Blur_Aim)))-1/N_H) >= 0.0){ //on suppose N_H le 1/current_distance  dec 2017 donc ici (1/(1/s-1/H)-s) = Df-s
			//Df=s(H-f)/(H-s) ou Dnf/(H-s)
			Df = Dnf2/(focus_calc_focal_H(Current_F, Current_aperture, sqrt(Defocus_Blur_Aim))-(Current_dist*10.0)); // in mm
			snprintf (inf_str, sizeof(inf_str), "%.3f", (Df-(Current_dist*10.0))/1000.0); // 1/(N_H-focus_calc_focal_N_H(Current_F, Current_aperture)) );//-1/N_H);
		}else{
			snprintf (inf_str, sizeof(inf_str), "inf");
		};
	};
		setcolor(RED);
		
		
		// diffraction aspect - 
		if (Defocus_Blur_Aim > 0) { // Ze_Current_Diff_blur < coc
			Current_x2_pix = (long)  (((N_last_left-(1000.0/Df)) /Nb_N_in_slot)*Nb_Pixel_in_slot)+Offset+X_Rule_Position; 
			if (Current_x2_pix>=Screen_width) Current_x2_pix = X_Rule_Position+Rule_Width; //+Offset;
		} else {
			Current_x2_pix=Screen_width/2;
			}; // else all is concerned but with a blur of Total_blur_delta-coc
			
		setcolor(YELLOW);
		
		
		 Current_x_pixel_far=Current_x_pixel;
		 Current_x_pixel_far_2 = Current_x2_pix;
		// end of diffraction block for far distance
	
	
	// near distance **********************************************************************************************
	setcolor(YELLOW);

	Dn = Dnf/(focus_calc_focal_H(Current_F, Current_aperture, coc)+(Current_dist*10.0)-2*Current_F); // in mm
	
	printf("to understand : Dnf : %f AND Dn %f \n", Dnf, Dn);
	printf("to understand : with coc %g : focal_H %f  Current_F %f \n", (double)coc, (double)focus_calc_focal_H(Current_F, Current_aperture, coc), (double)Current_F);
	
	if (Dn < 0.0) Dn=0.0;
		
	Dn1=Dn;
	 Current_x_pixel = (long)  (((N_last_left-(1000.0/Dn)) /Nb_N_in_slot)*Nb_Pixel_in_slot)+Offset+X_Rule_Position;

	if ((Current_x_pixel > 0)&&(Current_x_pixel<Screen_width)) 
		line (Current_x_pixel,ZeY,	Current_x_pixel,ZeY+Rule_height);

		if (Current_x_pixel <=0) Current_x_pixel = X_Rule_Position;
		
		setcolor(YELLOW); // Change red en yellow
		line (Current_x_pixel, ZeY-6+Rule_height/2,Screen_width/2 ,ZeY-6+Rule_height/2); //line of dof dist inside the rule
		//Dn=s(H-f)/(H+s-2f) ou Dnf/(H+s-2f)
		
			setfillstyle(SOLID_FILL, WHITE);
			bar (Screen_width/2 -60, ZeY-6+Rule_height/2,Screen_width/2-10, ZeY+6+Rule_height/2);
  		snprintf (inf_str, sizeof(inf_str), "<%.3f>", ((Current_dist*10.0)-Dn)/1000.0); //1/(N_H+focus_calc_focal_N_H(Current_F, Current_aperture)));// 1/N_H-   donc ici (s-1/(1/s+1/H))=s-Dn
			setcolor(BLACK); 
			outtextxy (Screen_width/2 -60, ZeY-6+Rule_height/2, inf_str);
			snprintf (inf_str, sizeof(inf_str), "%.3f", Dn/1000.0);
			setfillstyle(SOLID_FILL, BLACK);
			bar (Current_x_pixel-5, ZeY-20,Current_x_pixel + 40, ZeY-5);
			setcolor(YELLOW);setbkcolor(YELLOW);
			outtextxy(Current_x_pixel-5,ZeY-17, inf_str);

		NearDist=Dn;

		setfillstyle(SOLID_FILL, WHITE);

		setfillstyle(SOLID_FILL, BLACK);
		setcolor(YELLOW);

			
		// diffraction aspect - 

// diffraction display 
	if (Defocus_Blur_Aim > 0) {
		Dn = Dnf2/(focus_calc_focal_H(Current_F, Current_aperture, sqrt(Defocus_Blur_Aim))+(Current_dist*10.0)-2*Current_F); // in mm
		if (Dn < 0.0) Dn=0.0;
				
		snprintf (inf_str, sizeof(inf_str), "%.3f", ((Current_dist*10.0)-Dn)/1000.0); //1/(N_H+focus_calc_focal_N_H(Current_F, Current_aperture)));// 1/N_H-   donc ici (s-1/(1/s+1/H))=s-Dn
	};//else same value than dof
		setcolor(RED);
		// deleted 5fev2023 outtextxy(Screen_width/2-40, ZeY-5+Rule_height/2, inf_str);
		if (Defocus_Blur_Aim > 0) { // Ze_Current_Diff_blur < coc
			Current_x2_pix = (long)  (((N_last_left-(1000.0/Dn)) /Nb_N_in_slot)*Nb_Pixel_in_slot)+Offset+X_Rule_Position;
			//(int)(((N_last_left - (N_H+focus_calc_focal_N_H(Current_F, Current_aperture, sqrt(Defocus_Blur_Aim))))/Nb_N_in_slot)*Nb_Pixel_in_slot)+Offset+X_Rule_Position;
			if (Current_x2_pix <=0) Current_x2_pix = X_Rule_Position;
		} else {
			Current_x2_pix=Screen_width/2;
			}; // else all is concerned but with a blur of Total_blur_delta-coc
		setcolor(YELLOW);
		//not sure that it has a sense ZZZ diffraction YES  or NOT
		Current_x_pixel_near= Current_x_pixel;
		Current_x_pixel_near_2 = Current_x2_pix;
	// line (Current_x_pixel, ZeY-6+Rule_height/2,Current_x2_pix ,ZeY-6+Rule_height/2); 
		setcolor(RED);
		// end of diffraction block



		// afficher la part de diffraction en barre
	setfillstyle(SOLID_FILL, CYAN);
  bar (Screen_width/2,Max_y_ecran+10,Screen_width/2+100,Max_y_ecran+20);
  
  //RAZ the both bars
	setfillstyle(SOLID_FILL, BLACK);
  bar (Screen_width/2-100,Max_y_ecran+10,Screen_width/2,Max_y_ecran+40);
  bar (Screen_width/2+110,Max_y_ecran+10,Screen_width/2+250,Max_y_ecran+40);

  // calculate the true current coc with diffraction
  // Current_dist 
  // 
  
 // printf("Dn = %f Df = %f\n", Dn, Df);
 // This quantity, pronounced delta-v, is called the focus spread.
  Delta_v=(((NearDist*Current_F)/(NearDist-Current_F))-((FarDist*Current_F)/(FarDist-Current_F))); //units in mm // with defrac
  printf("NearDist = %g \n",NearDist);
  printf("FarDist = %g\n", FarDist);
  	if (Delta_v<0.0) Delta_v=0.0;

 printf("Delta_V = %g \n", Delta_v); 
			//units in mm Delta_v=vnear(vn)-vfar(vf) ;  1/u+1/v = 1/f (eq DoFinDepth n°5) => v= uf/(u-f)


	 //printf("ancien magnify %g", magnify);
	 magnify = Current_F/((Current_dist*10.0)-Current_F); // equation DoFinDepth n°8 : u-f=f/m => m = f/(u-f)
	 printf("-- magnify %g\n", magnify);

	 C_def = (Delta_v/(2*Current_aperture*(1+magnify))); 
	 C_diffr = (Current_aperture*(1+magnify)/Klambda);
	 C_T = sqrt((C_def*C_def)+(C_diffr*C_diffr));

if ((int)(10000*C_T)>(int)(10000*(cocx+0.00002))) {
	setcolor(RED); // change YELLOW in RED
	//Current_x_pixel_far ou _near
	line (Screen_width/2, ZeY-8+Rule_height/2 ,Current_x_pixel_far_2 ,ZeY-8+Rule_height/2); // décalage line from without diffraction (coc) until without diffraction
  line (Screen_width/2, ZeY-8+Rule_height/2,Current_x_pixel_near_2 ,ZeY-8+Rule_height/2);
  line (Screen_width/2, ZeY-4+Rule_height/2 ,Current_x_pixel_far_2 ,ZeY-4+Rule_height/2); // décalage line from without diffraction (coc) until without diffraction
  line (Screen_width/2, ZeY-4+Rule_height/2,Current_x_pixel_near_2 ,ZeY-4+Rule_height/2);
  };

	 printf("Current_F: %d\n",Current_F);
	 printf("Current Dist= %d centimeter --- NearDist: %0.4f meter FarDist %0.4f meter - DeltaV_near %f-DeltaV_far %f\n",Current_dist, (double)NearDist/1000.0,(double)FarDist/1000.0,(double)((NearDist*Current_F)/(NearDist-Current_F)),(double)((FarDist*Current_F)/(FarDist-Current_F)) );
	 printf("C_defocus : %g\n", C_def);
	 printf("C_diffraction : %g\n", C_diffr);
	 printf("Coc current Total : %g \n", C_T); 
	 printf("Coc camera cocx : %g \n", cocx);
	 printf("Coc defocus used for display : %g \n", coc);
	 //snprintf (inf_str, sizeof(inf_str), "Coc current: %f ", C_T); 

   show_bar_cocs ();


}

void display_Y_axis()
{
	
	
	if (F_max>F_min) { // not mono focal	
		for (int i=index_lens-1; i>=0; i--) {
			focal_line[i].pixel = (int)(Slote_aperture_line*(1.0/(double)(focal_line[i].length*focal_line[i].length)) + Offset_aperture_line);
			//if ((i==index_lens-1) || ((i<index_lens) && ((focal_line[i+1].pixel - focal_line[i].pixel) >= Rule_height))) {
				if (focal_line[i].pixel < last_rule_y_position || focal_line[i].pixel > last_rule_y_position + Rule_height){
					setcolor(WHITE);
					outtextxy (0, focal_line[i].pixel, focal_line[i].text);
					setcolor(DARKGRAY);
					line(X_Rule_Position, focal_line[i].pixel, Rule_Width+X_Rule_Position, focal_line[i].pixel);
				};
			//};
		};
	}; // end if (F_max>F_min)

}

void draw_Current_Y ()
{
	int current_pixel = last_rule_y_position;
	char inf_str[5];
	
	if (F_max>F_min)  // not mono focal	
		current_pixel = (int)((Slote_aperture_line*(1/(double)(Current_F*Current_F))) + Offset_aperture_line);
	else
		current_pixel = last_rule_y_position;
	

		setfillstyle(SOLID_FILL, WHITE);
		bar(0, current_pixel, 20, current_pixel+10);
		setfillstyle(SOLID_FILL, BLACK);

	setcolor(RED);
	//line(X_Rule_Position, current_pixel, Rule_Width, current_pixel); // it is the upper line of the rules in fact
	snprintf (inf_str, sizeof(inf_str), "%d", Current_F);
	outtextxy (0, current_pixel, inf_str);
	setfillstyle(SOLID_FILL, WHITE);
	//setcolor(WHITE);
	//bar (Rule_Width/2+X_Rule_Position, 0,Rule_Width/2+X_Rule_Position+25, 8);
	bar (0, current_pixel+Rule_height,30, current_pixel-10+Rule_height);
	snprintf (inf_str, sizeof(inf_str), "%.1f", Current_aperture);
	setcolor(RED);
	outtextxy (0, current_pixel+Rule_height-10,inf_str); //Rule_Width/2+X_Rule_Position, 0,inf_str);
 setfillstyle(SOLID_FILL, BLACK);

}
	
void draw_dof_values() //to display diffraction dist
{
	// BLACK (0), BLUE, GREEN, CYAN, RED, MAGENTA, BROWN, LIGHTGRAY, DARKGRAY,LIGHTBLUE, LIGHTGREEN, RED, LIGHTRED, RED, YELLOW, WHITE};
	
 	double N_fd = N_H, My_N;
	// I.calculate pixel position 
	long Current_x_pixel, Old_pixel, My_pixel;
	long last_x_l =0, last_x_r=0;
	char Dist_str[8];
	int to_display_current=1;

	//display values of apertures 0_9 -values_aperture_full[i] in a color ; one for each same aperture
	// and by pixel the values of 1/H on the line of Current_F and aperture 
	//setfontcolor(YELLOW);
	//setcolor(RED);
		
	//  values aperture of the current focal on upper line and up of the full stick
	// the rule is put on current focal (normally)
	
	// proposition : draw lines between each same aperture with the color of this aperture 
	// can be a single one between the fmax and fmin
	// to not display values except for the full aperture in the current F
	// to make stick for full aperture
	// at the current focal : set stick in 1/2 and 1/3 at different size
	
// for the linked line : normally we can make only one line to F_min to F_max

	for (int i = 0; i < nb_apertures; i++) 
//	for (int i = nb_apertures-1; i>=0; i--)
	{
	last_x_l =0; last_x_r=0;to_display_current=1;
	//printf("Aperture=%f\n", values_aperture_full[i]);
	
		if ((values_aperture_full[i]>= aperture_min)&&(values_aperture_full[i]<= aperture_max)) { // if aperture is possible with the actual lens

			setcolor(i+1); // to refer to graphics color : to vary the color per aperture
			
			for (int j=0;j<index_lens;j++)  { // index_lens must be zero if a one value lens
				My_N = (N_fd+HDs[i][j]);
				My_pixel = focal_line[j].pixel;
				if (focal_line[j].pixel >= last_rule_y_position && focal_line[j].pixel <= last_rule_y_position + Rule_height) continue;

				if (focal_line[j].length == Current_F) to_display_current=0;
				if ((Current_F < focal_line[j].length) && (to_display_current==1))
				{
					My_N = (N_fd+Currents_Focal_HD[i]);
					 
					My_pixel = (long)(Slote_aperture_line*(1.0/(double)(Current_F*Current_F)) + Offset_aperture_line);
				};
				Current_x_pixel = (long)(((N_last_left - My_N)/Nb_N_in_slot)*Nb_Pixel_in_slot)+Offset+X_Rule_Position;
				if ((Current_x_pixel >= 0)&&(Current_x_pixel<Screen_width)) {
					mini_triangle (Current_x_pixel, My_pixel);
					if (last_x_l > 0) // linked line
						line(last_x_l, Old_pixel, Current_x_pixel, My_pixel);
					last_x_l = Current_x_pixel;
				}; // end (Current_x_pixel > 0)

				My_N = (N_fd-HDs[i][j]);			
				if ((Current_F < focal_line[j].length) && (to_display_current==1))
				{
					to_display_current=0;
					My_N = (N_fd-Currents_Focal_HD[i]);
					j--;
				};
				// print aperture point to the right side
				Current_x_pixel = (long)(((N_last_left - My_N)/Nb_N_in_slot)*Nb_Pixel_in_slot)+Offset+X_Rule_Position;
				if ((Current_x_pixel >= 0)&&(Current_x_pixel<Screen_width)) {
					mini_triangle (Current_x_pixel, My_pixel);
					if (last_x_r > 0) // linked line
						line(last_x_r, Old_pixel, Current_x_pixel, My_pixel);
					last_x_r = Current_x_pixel;
				}; // end (Current_x_pixel > 0)
				Old_pixel = My_pixel;
			}; // end for j<index_lens

			if (to_display_current==1) {
			Current_x_pixel = (int)(((N_last_left - (N_fd+Currents_Focal_HD[i]))/Nb_N_in_slot)*Nb_Pixel_in_slot)+Offset+X_Rule_Position;
			if ((Current_x_pixel >= 0) && (Current_x_pixel<Screen_width)) {
				mini_triangle (Current_x_pixel, (int)(Slote_aperture_line*(1.0/(double)(Current_F*Current_F)) + Offset_aperture_line));
				//snprintf (Dist_str, sizeof(Dist_str), "%.2f", (1.0/(N_fd+Currents_Focal_HD[i])) ); // we can format if we want to limit (with sscanf) the printing value				
			}; // end if (Current_x_pixel > 0) {

			My_N = (N_fd-Currents_Focal_HD[i]);
			Current_x_pixel = (int)(((N_last_left - My_N)/Nb_N_in_slot)*Nb_Pixel_in_slot)+Offset+X_Rule_Position;
			
			if ((Current_x_pixel >= 0)&&(Current_x_pixel<Screen_width)) {
				mini_triangle (Current_x_pixel, (int)(Slote_aperture_line*(1.0/(double)(Current_F*Current_F)) + Offset_aperture_line));
			}; // end if (Current_x_pixel > 0) 		
			};
		}; // end if ((values_aperture_full[i]>= aperture_min)&&(values_aperture_full[i]<= aperture_max))
	} ; // end for i < nb_apertures


// half values
	setcolor(YELLOW); // to refer to graphics color : to vary the color per aperture
	//setfontcolor(YELLOW);

	for (int i = 0; i < sizeof(values_aperture_demi)/sizeof(values_aperture_demi[0]); i++) {
		if ((values_aperture_demi[i]>= aperture_min)&&(values_aperture_demi[i]<= aperture_max)) { // if aperture is possible with the actual lens
			// left side
		Current_x_pixel = (int)(((N_last_left - (N_fd-focus_calc_focal_N_H(Current_F, values_aperture_demi[i], coc)))/Nb_N_in_slot)*Nb_Pixel_in_slot)+Offset+X_Rule_Position;
			if ((Current_x_pixel > 0)&&(Current_x_pixel<Screen_width)) { //medium stick upside for 1/2 value --> must be overwrite the plain one then in grey ?
				line (Current_x_pixel, Slote_aperture_line*(1/(double)(Current_F*Current_F)) + Offset_aperture_line-6, Current_x_pixel, Slote_aperture_line*(1/(double)(Current_F*Current_F)) + Offset_aperture_line); //medium stick upside for plain value
			// right side
			}; // if (Current_x_pixel > 0)
			Current_x_pixel = (int)(((N_last_left - (N_fd+focus_calc_focal_N_H(Current_F, values_aperture_demi[i],coc)))/Nb_N_in_slot)*Nb_Pixel_in_slot)+Offset+X_Rule_Position;
			if ((Current_x_pixel > 0)&&(Current_x_pixel<Screen_width)) { //medium stick upside for 1/2 value --> must be overwrite the plain one then in grey ?
				line (Current_x_pixel, Slote_aperture_line*(1/(double)(Current_F*Current_F)) + Offset_aperture_line-6, Current_x_pixel, Slote_aperture_line*(1/(double)(Current_F*Current_F)) + Offset_aperture_line); //medium stick upside for plain value
			}; // if (Current_x_pixel > 0)
		}; // if ((values_aperture_demi[i]>= aperture_min)
	}; // for (init i = 0; i < length(values_aperture_demi)
		
// 1/3 values
	setcolor(RED); // to refer to graphics color : to vary the color per aperture
	//setfontcolor(RED);
	for (int i = 0; i < sizeof(values_aperture_tiers)/sizeof(values_aperture_tiers[0]); i++) {
		if ((values_aperture_tiers[i]>= aperture_min)&&(values_aperture_tiers[i]<= aperture_max)) { // if aperture is possible with the actual lens
			// left side
			Current_x_pixel = (int)(((N_last_left - (N_fd-focus_calc_focal_N_H(Current_F, values_aperture_tiers[i],coc)))/Nb_N_in_slot)*Nb_Pixel_in_slot)+Offset+X_Rule_Position;
			if ((Current_x_pixel > 0)&&(Current_x_pixel<Screen_width)) { //medium stick upside for 1/2 value --> must be overwrite the plain one then in grey ?
				line (Current_x_pixel, Slote_aperture_line*(1/(double)(Current_F*Current_F)) + Offset_aperture_line-3, Current_x_pixel, Slote_aperture_line*(1/(double)(Current_F*Current_F)) + Offset_aperture_line); //medium stick upside for plain value
			// right side
			}; // if (Current_x_pixel > 0)
			Current_x_pixel = (int)(((N_last_left - (N_fd+focus_calc_focal_N_H(Current_F, values_aperture_tiers[i],coc)))/Nb_N_in_slot)*Nb_Pixel_in_slot)+Offset+X_Rule_Position;
			if ((Current_x_pixel > 0)&&(Current_x_pixel<Screen_width)) { //medium stick upside for 1/2 value --> must be overwrite the plain one then in grey ?
				line (Current_x_pixel, Slote_aperture_line*(1/(double)(Current_F*Current_F)) + Offset_aperture_line-3, Current_x_pixel, Slote_aperture_line*(1/(double)(Current_F*Current_F)) + Offset_aperture_line); //medium stick upside for plain value
			}; // if (Current_x_pixel > 0)
		}; // if ((values_aperture_tiers[i]>= aperture_min)
	}; // for (init i = 0; i < length(values_aperture_tiers)

} // end draw_dof_values function


// try to init values that not change always to improve perf.
void init_Focals_HD ()
{
	int first_focal_reached = 0, last_lens_reached=0;
	// must do make special case for macro lens ?

	// to have a straight line between all same aperture values on different focal values
	// we have then to have an equation y=Slote_aperture_linex+Offset_aperture_line between Y axis and X axis
	// X axis is on 1/H approxiamtively thus exactly Nc/f2
	// Y axis is based then on 1/f2 and we put Y axis for a lens with different possible values with the following principles
	// put the focal min at the top then pixel 10 (to display on upper line of the rules, the H and current distance values) and 
	// F_min is larger distance range than F_max
	// put the focal max on the and current distance on the upper line of the rules
	//in assumption where we ahave 480x 720 définition but we can have more
	// pixel_10= Slote_aperture_line(1/F_min2) + Offset_aperture_line ; pixel_430 = Slote_aperture_line(1/F_max2) + Offset_aperture_line (430=Max_y_ecran-rule_height)
	// to know the YNb_Pixel_in_slot => pixel_430-pixel_10=430=(1/F_max2-1/F_min2)Slote_aperture_line
	// Slote_aperture_line = 430-10/(1/F_max2-1/F_min2)
	// Offset_aperture_line = 430-10-Slote_aperture_line(1/F_max2)
	// Pixels values of ith F (Fi) = Slote_aperture_line(1/Fi2) + Before_comma

	index_lens=0;
	
	if (F_max>F_min) { //  multi-value lens

	//focals must be initialized with min and max focal of the current lens
			for (int k = 0; k<sizeof(focals)/sizeof(focals[0]);k++) { // max about 12 lines on Y axis in addition of the current one
				if (focals[k] == F_min) {first_focal_reached = 1; focal_line[0].length=focals[k];index_lens=1;
										 sprintf (focal_line[0].text,"%d", focals[k]);
										 } else 
				if ((focals[k] > F_min) && (focals[k] < F_max))
					if (first_focal_reached == 0) {first_focal_reached = 1;
						focal_line[0].length=F_min;focal_line[1].length=focals[k]; index_lens =2; 
						sprintf (focal_line[0].text, "%d", F_min);
						sprintf (focal_line[1].text, "%d", focals[k]);
						} 
					else {focal_line [index_lens].length=focals[k];
						   sprintf (focal_line[index_lens].text,"%d", focals[k]);
						   index_lens ++;}
				else
				if (focals[k] == F_max) {
					last_lens_reached=1;
					focal_line [index_lens].length=focals[k];
					sprintf (focal_line[index_lens].text, "%d", F_max);
					index_lens ++;
					} else
				if (focals[k] > F_max)	
					if (last_lens_reached==0) {
						last_lens_reached=1;
						focal_line [index_lens].length=F_max;
						sprintf (focal_line[index_lens].text, "%d", F_max);
						index_lens ++;
						break;
					};
			
			}; // end for int k = 0
	}; // end if (F_max>F_min)
			
	for (int i=0; i<nb_apertures; i++) { //apertures	
	// to adapt with aperture_min and aperture_max of the current lens 
	if ((values_aperture_full[i]>= aperture_min)&&(values_aperture_full[i]<= aperture_max)) {
		if (F_max>F_min) //  multi-value lens
			for (int j =0;j<index_lens;j++)  // focals
				HDs[i][j]=focus_calc_focal_N_H(focal_line[j].length,values_aperture_full[i],coc);				
		Currents_Focal_HD[i] = focus_calc_focal_N_H(Current_F, values_aperture_full[i],coc);
		//Currents_Diff [i] = (values_aperture_full[i]*2.44*(lgonde/1000.0))/diag; // not necessary
	}; // end aperture_min & aperture_max
	}; // end for (int i=0; i<nb_apertures; i++)
} // end init_Focals_HD


void display_cadre()
{
//setfontcolor(YELLOW);
setcolor(RED);
line (0,Max_y_ecran+1,Screen_width+1,Max_y_ecran+1);
line (Screen_width+1,0,Screen_width+1,Max_y_ecran+1);
}


void refresh_screen ()
{
	char Apert_str[5];
	
	cleardevice(); //  can be adapt/improve to the rule only depending to the action made -- scroll or zoom or move rule
	setbkcolor(BLACK);
	display_cadre(); // to adapt delete when implement on the target screen
	

		// call the next functions only if zoom or move demand (we can update only changed values if mode after)
	display_Y_axis ();

	draw_dof_values();
	for (int i = 0; i < nb_apertures; i++) 	
		if ((values_aperture_full[i]>= aperture_min)&&(values_aperture_full[i]<= aperture_max)) { // if aperture is possible with the actual lens
			setcolor(i+1); // to refer to graphics color : to vary the color per aperture
			//setfontcolor(i+1);
			snprintf (Apert_str, sizeof(Apert_str), "%.1f", values_aperture_full[i]);
			outtextxy (Screen_width/2 -30 - (i*30), 0, Apert_str) ;
			outtextxy (Screen_width/2 +30 + (i*30), 0, Apert_str) ;
		};
	//setcolor(WHITE);
	//setfontcolor(YELLOW);
	setcolor(WHITE);
	line (Screen_width/2, 15, Screen_width/2, Max_y_ecran);
	draw_dof_rule(); 
	draw_Current_Y();
//		draw_Current_Y();

	draw_current_aperture();
}

void refresh_rule ()
{
	
	//setcolor(WHITE);
	setcolor(WHITE);
	line (Screen_width/2, 15, Screen_width/2, Max_y_ecran);
	setbkcolor(BLACK);
	draw_dof_rule(); 
	draw_Current_Y();
	draw_current_aperture();
}

void Scroll_HR (double Shift) // Shift N pixels on the right : means also a shift in the focus distance
{
	double delta = ((Shift*Nb_N_in_slot)/Nb_Pixel_in_slot);
	if (Offset <  (Rule_Width/2)) {
		if (Offset == 0) {
			if (N_last_left == Min_dist) {
				Offset += (long)Shift;
				delta = 0.0;
			} else {
			if ((N_last_left+delta)<=Min_dist) {
				N_last_left+=delta;
			}else{
				// > Min_dist est-ce possible si on va vers la droite ?? 
				//ex: N_last_left = 9.8, delta = 1/2 le nouveau N_last_left = Min_dist (max possible 10.0 pour 0.1m) - 
				// new_delta/Offset = Min-dist-N_last_left =10.0-9.8=0.2 - Offset = old_delta-new_delta = 0.5-0.2=0.3
				Offset = (long)((delta-(Min_dist-N_last_left))*Nb_Pixel_in_slot/Nb_N_in_slot);
				delta = Min_dist-N_last_left;
				N_last_left = Min_dist;
			};
		};
		if 	((N_first_right+delta)>= 0.0) 
				N_first_right+=delta;

		} else  // Offset is already not null
			if ((Offset <0) && (Offset+Shift) >0) { // change of sign
				delta = (((Offset+Shift)*Nb_N_in_slot)/Nb_Pixel_in_slot);
				Offset=0;
				N_first_right+=delta;
				N_last_left+=delta;
			}else{
			Offset += (long)Shift;
			fprintf(stderr, "ici+20=%d\n", Offset);
			if (Offset >= (Rule_Width/2)) 
					Offset= (Rule_Width/2);
			};		
		if (Offset >= (Rule_Width/2)) {
			N_H = 100.0/Min_dist;
			Current_dist = Min_dist;
			fprintf(stderr, "atteint! min\n");
		}else {
		if (Offset != 0)
			delta = ((Offset*Nb_N_in_slot)/Nb_Pixel_in_slot);
		else
			delta = 0.0;
		N_H = ((N_last_left+delta-((double)((Rule_Width/2.0))/Nb_Pixel_in_slot)*Nb_N_in_slot)); // 
		Current_dist = (long)(100.0/N_H);
		fprintf (stderr, "CD-right=%d---Toto=%.10f---Pixel=%d\n", Current_dist, N_H, (long)(((N_last_left - N_H)/Nb_N_in_slot)*Nb_Pixel_in_slot));
		};
		
		refresh_rule(); // refresh_rule only is best with bmp_raz of space between the two lines 
	};
	
}

void Scroll_HL (double Shift) // Shift N pixels on the left : means also a shift in the focus distance
{ // hypothèse shift est positif
	double delta = ((Shift*Nb_N_in_slot)/Nb_Pixel_in_slot);	// delta is in unit (i.e. meter)		
	fprintf(stderr, "Scroll_H-Offset=%d\n", Offset);
	
	if (Offset > -1*(Rule_Width/2)) { // not reached a limit ?
		if (Offset == 0) { //the window is included : not blank part (cas B)
			if (N_first_right==0.0) { // infinity
				Offset -= (long)Shift; // create blank part
				delta = 0.0;
			} else {	
			if ((N_first_right-delta)>= 0.0) { // until the right part is not blank part 
				N_first_right-=delta; 
			}else{ // <0 est-ce possible si on va vers la gauche ?? ex: first_right = 1/8, delta = 1/2 le nouveau first_right = 0 (min possible) - delta/Offset = 1/8 - 3/8
				fprintf(stderr, "cas N_first_right-delta)>= 0.0)\n"); // passe de A vers C
				Offset = (long)((N_first_right-delta)*Nb_Pixel_in_slot/Nb_N_in_slot);
				delta = N_first_right; 
				//N_last_left -=N_first_right;
				N_first_right=0.0;
				// make the mixte part 
				};
			};
			if ((N_last_left-delta)<=Min_dist) {// que faire si non ? 
				N_last_left-=delta;
			}else {fprintf(stderr,"N_last_left-delta");};
			
		} else { //Offset not null 
		// can be changed of sign
			if ((Offset >0) && (Offset-Shift) <0) { // change of sign --> recaler car on ne peut pas avoir
				delta = (((Shift-Offset)*Nb_N_in_slot)/Nb_Pixel_in_slot);
				Offset=0;
				N_first_right-=delta;
				N_last_left-=delta;
			}else{ // Offset <=0 ou Offset-Shift >=0
			Offset -= (long)Shift;
			fprintf(stderr, "ici-20Off%d\n", Offset);
			if (Offset <= -1*(Rule_Width/2)) 
					Offset= -1*(Rule_Width/2);
			};
		
		}; //blank in the left
		
		
		// To update Current_dist and N_H
				
		if (Offset <= -1*(Rule_Width/2)) { // limit reached
			Offset = -1*(Rule_Width/2);
			Current_dist = 10 * 1000 * 1000; // infinity
			magnify=0;
			N_H=0.0;
			fprintf(stderr, "atteint infiny\n");
		} else { // Offset > -1*(Rule_Width/2)
			if (Offset != 0)
				delta = ((Offset*Nb_N_in_slot)/Nb_Pixel_in_slot);
			else delta=0.0;
			
			N_H = ((N_last_left+delta-((double)((Rule_Width/2.0))/Nb_Pixel_in_slot)*Nb_N_in_slot));
			Current_dist = (long)(100.0/N_H);
			fprintf (stderr, "CD-left=%d---Nb_Pixel_in_slot=%.10f--Nstep=%.10f---Pixel=%d\n", Current_dist, Nb_Pixel_in_slot, Nb_N_in_slot, (long)(((N_last_left - N_H)/Nb_N_in_slot)*Nb_Pixel_in_slot));
			};
			
		refresh_rule(); // refresh_rule only is best with bmp_raz of space between the two lines 
		
	 };
	 
	 
}

void Scroll_V (int C_F)
{
		if ((C_F >= F_min) && (C_F <= F_max)) {
			Current_F = C_F;
			last_rule_y_position = Slote_aperture_line*(1.0/(double)(C_F*C_F)) + Offset_aperture_line;
			for (int i=0; i<nb_apertures; i++) 
				if ((values_aperture_full[i]>= aperture_min)&&(values_aperture_full[i]<= aperture_max)) 
				Currents_Focal_HD[i] = focus_calc_focal_N_H(C_F, values_aperture_full[i],coc);
			refresh_screen();
		};
}

void Zoom (double Coeff)
{
	long final_pixel, orig_pixel;

		if (Offset==0) {
				N_first_right+=(Coeff*Nb_N_in_slot); // à traiter cas infini
				if (N_first_right < 0.0) N_first_right = 0.0;
			
				N_last_left-=(Coeff*Nb_N_in_slot); // à traiter Min Dist
				if (N_last_left > Min_dist) N_last_left = Min_dist;
			//printf("nbr--slot=%d - Nb_N_in_slot=%f\n", nbr_slot_intertick, Nb_N_in_slot);
			Nb_N_in_slot=(N_last_left-N_first_right)/(nbr_slot_intertick); // slot by accuracy (init 1/100 or 0.01)
			// traiter cas où fist_right negatif et last_left =min_dist 
						
		}else{ // Offset != 0
			if (N_first_right == 0.0) { // infinity and offset means inside with blank at right part
				orig_pixel = (long)(((N_last_left - N_H)/Nb_N_in_slot)*Nb_Pixel_in_slot);
				N_last_left -=(2.0*Coeff*Nb_N_in_slot); 
				Nb_N_in_slot=(N_last_left-N_first_right)/(nbr_slot_intertick);
				final_pixel = (long)(((N_last_left - N_H)/Nb_N_in_slot)*Nb_Pixel_in_slot);
				fprintf(stderr, "First==0 & Offset avant=%d\n", Offset);
				if (Offset > -1*(Rule_Width/2)) 
					Offset -=  (long) (final_pixel-orig_pixel);
				if (Offset > 0) {// normalement un N_first_right à zéro et un Offset non nul signifie un Offset négatif
								// ici c'était pour "recaler" cela
					fprintf(stderr, "Offset apres=%d\n", Offset);
					N_first_right = ((Offset*Nb_N_in_slot)/Nb_Pixel_in_slot);
					N_last_left+=N_first_right;
					Nb_N_in_slot=(N_last_left-N_first_right)/(nbr_slot_intertick);
					Offset=0;
				};// end if (Offset > 0)
				fprintf (stderr, "In Zoom : Offset-->%d<-- et Diff pixel=%d\n", Offset, Nb_Pixel_in_slot, final_pixel-orig_pixel);
			} else { // N_last_left=Min_dist -- means pb of blank on left side with possibility of a focus on Min_dist 
				fprintf(stderr, "avant First_right=%f   Nstep=%f\n", N_first_right, Nb_N_in_slot);
				N_first_right+=(2.0*Coeff*Nb_N_in_slot);
				orig_pixel = (long)(((N_last_left - N_H)/Nb_N_in_slot)*Nb_Pixel_in_slot);
				fprintf(stderr, "orig_pixel=%d\n", orig_pixel);
				Nb_N_in_slot=(N_last_left-N_first_right)/(nbr_slot_intertick);
				fprintf(stderr, "après First = %f  Nstep=%f\n", N_first_right, Nb_N_in_slot);
				final_pixel = (long)(((N_last_left - N_H)/Nb_N_in_slot)*Nb_Pixel_in_slot);
				fprintf(stderr, "final_pixel=%d\n", final_pixel);
				fprintf(stderr, "Offset av %d\n", Offset);
				if (Current_dist != Min_dist)
					Offset -=  (long)(final_pixel-orig_pixel);
				fprintf(stderr, "Offset ap %d\n", Offset);
				if (Offset < 0) {
					N_last_left+=((Offset*Nb_N_in_slot)/Nb_Pixel_in_slot);
					if (N_last_left > Min_dist) N_last_left = Min_dist;
					N_first_right-=((Offset*Nb_N_in_slot)/Nb_Pixel_in_slot);
					if (N_first_right<0.0) N_first_right=0.0;
					Nb_N_in_slot=(N_last_left-N_first_right)/(nbr_slot_intertick);
					Offset=0;
				}
			};
		};	
			N_H = ((N_last_left-((double)((Rule_Width/2.0)-Offset)/Nb_Pixel_in_slot)*Nb_N_in_slot));
			Current_dist = (long)(100.0/N_H);
		refresh_screen ();
}
void GoToDist2 (double MyDist)
{
		double empatement_in_N = (((Rule_Width/2)/Nb_Pixel_in_slot)*Nb_N_in_slot);
		Current_dist = (long)(MyDist*100.0);
		N_H = 1.0/MyDist;
		N_last_left = 	N_H+empatement_in_N;
		N_first_right = N_H-empatement_in_N;
		Offset=0;
		fprintf (stderr, "GoToDist2 : %d ", MyDist);
		if (N_last_left >= Min_dist) { // assume only one blank on the left or exclusivly on the right
			Offset = (long)(((N_last_left-Min_dist)/Nb_N_in_slot)*Nb_Pixel_in_slot); // positif
			fprintf(stderr, "N_last_left=%f, Off=%d\n", N_last_left, Offset);
			N_last_left = Min_dist;
			N_first_right +=(N_last_left-Min_dist);
		};
		
		if (N_first_right<=0.0) {
			Offset = (long)((N_first_right/Nb_N_in_slot)*Nb_Pixel_in_slot); // doit etre Negatif
			fprintf(stderr, "N_first=%f ** Offset =%d |\n", N_first_right, Offset);
			N_last_left-=N_first_right;
			N_first_right = 0.0;
			
		};
		magnify = Current_F/((Current_dist*10.0)-Current_F); //all units in mm

	  refresh_coc ();
	  
		refresh_screen ();
}

void GoToDist (double MyDist)
{
	long Current_x_pixel1 = (long)(((N_last_left - (1.0/MyDist))/Nb_N_in_slot)*Nb_Pixel_in_slot)+Offset+X_Rule_Position; 
	long Current_x_pixel2 = (long)((double)((N_last_left - (100.0/Current_dist))/Nb_N_in_slot)*Nb_Pixel_in_slot)+Offset+X_Rule_Position; 
	
	double Glisse = (double)(Current_x_pixel2 - Current_x_pixel1);
	fprintf(stderr, "[GoToDist] MyDist=%f, N_H=%d Glisse=%f - CCGoal=%d CCurent=%d\n",  MyDist, Current_dist, Glisse, Current_x_pixel1, Current_x_pixel2);
	// 2m vers 5.0 m = 1/5- 1/2 m = 0.2- 0.5 <0
	if (Glisse <0) {
		fprintf(stderr, "à HL=%d\n", Glisse);
		Scroll_HL(-1.0*Glisse);
	}else {
		Scroll_HR(Glisse);
		fprintf(stderr, "à HR=%d\n", Glisse);
	};
}

int ReadEntier (int to_show)
{
	char c_int='1';
	int int_to_return =0;
	char str_to_display[10];
	
	while ((int)c_int != 13) 
	{
	  c_int = readkey();
	  if (c_int >= '0' && c_int <= '9')  
	  {
	  	int_to_return = int_to_return * 10 + atoi(&c_int);
	  	if (to_show) {
	  		snprintf (str_to_display, sizeof(str_to_display), "%d", int_to_return); 
	  		outtextxy(270, 370, str_to_display);
	  		};
	  };
	}
	return int_to_return;
}

int Capture_entier(char *MyMessage)
{
	char Texte[50];

		setfillstyle(SOLID_FILL, WHITE);
		bar(250, 250, 700, 400);
		snprintf (Texte, sizeof(Texte), MyMessage);
		setcolor(BLACK);
		outtextxy(270, 350, Texte);
		return ReadEntier (1);
}

void MyAlert (char *MyMessage, int line, int init)
{
	  char Texte[50];
	  if (init) {
		 setfillstyle(SOLID_FILL, WHITE);
		 bar(250, 250, 700, 400);
		};
		
		snprintf (Texte, sizeof(Texte), MyMessage);
		setcolor(BLACK);
		outtextxy(270, 350+10*line, Texte);
		readkey();
}

int Init ()
{

char Answer[2];

printf("which the lower focal (example 15,35, 250, ...) ? \n");
fscanf(stdin, "%d", &F_min);
printf("which the upper focal ? \n");
fscanf(stdin, "%d", &F_max);
printf("where set the current focal ? \n");

fscanf(stdin, "%d", &Current_F);

	printf( "\n");
  printf("    by default we take into account in the equation diffraction+defocus then only yellow lines\n");	
	printf( "*** DOF : if Red lines : \n");
	printf( "             red lines is defocus+diffraction blur to reach camera coc;\n");
	printf( "             yellow lines without taking account diffraction\n") ; 
	printf( "|**************************************************************************|\n");
	printf( "|***COMMANDS to type in graphical screen and values in command terminal****|\n");
	printf( "|**************************************************************************|\n");
	printf( "|m_ou_l -more ou less sticks                                               |\n");
	printf( "|a-to choose near&far distance                                             |\n");
	printf("|i-to refresh                                                              |\n");
	printf( "|g-set a distance                                                          |\n");
	printf( "|c-set a coc, 0,019mm by default                                           |\n");
	printf( "|           |           /|\\                                                |\n");
	printf( "|d(own) or \\|/ and u(p)  | to change the focal                             |\n");
	printf( "|o set aperture                                                            |\n");
	printf( "|z or '+' for zoom and y or '-' for unzoom                                 |\n");
	printf( "|left arrow '<--' or l(eft); '-->' or r(ight) change dist at left, at right|\n");
	printf( "|s to switch total coc is only defocus or with diffraction                 |\n");
	printf( "|h to set hyperfocal  distance                                             |\n");
	printf( "|**************************************************************************|\n");
	printf( "\n");


printf("in Full Screen or not ? y/n\n");

fscanf(stdin, "%s", &Answer);
printf(" reponse %s\n", &Answer[0]);

//int gdriver=DETECT, gmode=0;
int gdriver=VGA, gmode= GM_1024x768; //GM_800x600;
char c_pause;
	double delta;
	int j=0;
	char Alert [50];
	//int MyDist;
	

if ((Answer[0] == 'y') || (Answer[0] == 'Y')) {

	 initgraph(&gdriver, &gmode, "FULL_SCREEN");
	} else {
	 initgraph(&gdriver, &gmode, "");
};

		 // "RGBFULL_SCREEN");

  
  getpalette(&pal);

  setrgbpalette(pal.colors[0], 0, 0, 0); // black
	setrgbpalette(pal.colors[1], 0, 0, 255); // blue
	setrgbpalette(pal.colors[2], 0, 255, 0); // green
	setrgbpalette(pal.colors[3], 0, 255, 255); // cyan
	setrgbpalette(pal.colors[4], 255, 0, 0); // RED
	setrgbpalette(pal.colors[5], 255, 0, 255); // Magenta
	setrgbpalette(pal.colors[6], 139,69,19); // brown
	setrgbpalette(pal.colors[7], 200, 200, 200); // _LIGHTGRAY
	setrgbpalette(pal.colors[8], 128, 128, 128); // _DARKGRAY
	setrgbpalette(pal.colors[9], 100, 100, 255); // _LIGHTBLUE
	setrgbpalette(pal.colors[10], 200, 255, 255); // _LIGHTCYAN
	setrgbpalette(pal.colors[11], 250, 100, 100); // _LIGHTRED
	setrgbpalette(pal.colors[12], 255, 255, 0); //_YELLOW
	setrgbpalette(pal.colors[13], 255, 255, 255); // _WHITE


	setfillstyle(SOLID_FILL, BLACK);
	
  init_rule_dof();

	refresh_coc ();

  //init_Focals_HD(); // already inside refresh_coc
  
  refresh_screen();

  c_pause='i'; 

	fprintf(stderr, "\n");

while (c_pause != 'q')
{

	
	j++;
	
	if (c_pause == 'i')	{refresh_screen();}; //init
	
	if (c_pause == 'g') {
		int MyDist;
		
		MyDist = Capture_entier ("type distance in cm");
		//fprintf(stderr, "coucou=%f\n", (double)(MyDist/100.0));
		GoToDist2((double)(MyDist/100.0));
	};
	
	if (c_pause == 'a') {
		int NearDist, FarDist, MyDist;
		double Discriminant, X1, X2;
		double Aper1, Aper2, New_aperture;
		int NewFocal, BestFocal;
		double Best_coc,  NewDelta_v, Newmagnify;
		
		
	  NearDist=Capture_entier ("type near distance in cm");
	  FarDist=Capture_entier ("type far distance in cm");
		
		///calculate of an ideal
		// remember : The object at distance u is real object in focus, the corresponded projected image is with distance v ; see figure 1 page 5
		// see also http://www.georgedouvos.com/douvos/OptimumCS-Pro_Optical_Science.html
		// have an impact on magnify (eq. DoFinDepth n°30 extrapoled to u ot=r deduced from n°29) NearDist ud and FarDist = uf ;
		MyDist = (int)(2*NearDist*FarDist)/(NearDist+FarDist); 
		
		
		//units in mm =vf-vn ;  1/u+1/v = 1/f (eq DoFinDepth n°5) => v= uf/(u-f)
		Delta_v=(((10.0*NearDist*Current_F)/((10.0*NearDist)-Current_F))-((FarDist*10.0*Current_F)/((FarDist*10.0)-Current_F))); 
		magnify = Current_F/((MyDist*10.0)-Current_F); // equation DoFinDepth n°8 : u-f=f/m => m = f/(u-f)
		
		// normally multiply by 1/(1+m) m is the magnification, 0 is at infinity
		// It is N_Min corresponding to eq (114) of Conrad at http://www.largeformatphotography.info/articles/DoFinDepth.pdf
		Current_aperture=(double)(sqrt((Klambda/2.0)*Delta_v))/(1+magnify); //eq 114


		snprintf(Alert, sizeof(Alert), "Aperture Conrad %.2f, closest=%.1f********", Current_aperture, the_closest_aperture(Current_aperture));
		MyAlert(Alert, 0,1);
		
		Current_aperture = the_closest_aperture(Current_aperture);
		
		
		// another test to have the possible N, with fixed coc, Delta_V, lambda and F (proposal if possible)
		// is it relative to eq 113 : ct (or coc)=sqrt( square[Delta_v/2*N*(m+1)] + square[N*(m+1)/Klambda] ) 
		/* then we derive this equation indicating the variance and search the N when it is zero (means that we reach the head of the curve)
		derivated equation : sqr(DeltaV)/(4*sqr(N)*sqr(1+m)) + sqr(N) * sqr(1+m)/sqr(Klambda)
		etc.
		*/
		printf ("Actual/Real current circle of total blur %g\n", sqrt(Delta_v/Klambda));

	if (sqrt(Delta_v/Klambda) > cocx)	{
				snprintf(Alert, sizeof(Alert), "Blur is greater than declared camera coc %.3f", cocx);
				MyAlert(Alert, 1,0);

			// in http://www.largeformatphotography.info/articles/DoFinDepth.pdf, page 28, after eq 114, Conrad indicates that the optimum diameter of blur (which minimizes the blur) indicates
			// kdiffraction = kdefocus and =  sqrt(Delta_v/2*Klambda) is optimum
			// as kTotal*kTotal=kdiff*kdiff + kdefocus*kdefocus  (eq 112) then kTotal*kTotal = 2 * Delta_v/2*Klambda then kTotal = sqrt(Delta_v/Klambda) is optimum and we search that it reach inside our coc
	
			
		//
		// F=10 is always the best ...
		//
			Best_coc = cocx;
			BestFocal=0;
			for (NewFocal=Current_F-1;NewFocal>10;NewFocal--) {
				printf("iteration %d\n",NewFocal);
				NewDelta_v=(((10.0*NearDist*NewFocal)/((10.0*NearDist)-NewFocal))-((FarDist*10.0*NewFocal)/((FarDist*10.0)-NewFocal))); 
				if (sqrt(NewDelta_v/Klambda) <= Best_coc) {Best_coc=sqrt(NewDelta_v/Klambda);BestFocal=NewFocal;};
				};
				if (BestFocal != 0) {
				Newmagnify = BestFocal/((MyDist*10.0)-BestFocal); // equation DoFinDepth n°8 : u-f=f/m => m = f/(u-f)
				New_aperture=(double)(sqrt((Klambda/2.0)*NewDelta_v))/(1+Newmagnify);
			};
			printf("test Focal best Best-coc=%g at best_F=%d\n",Best_coc, BestFocal);

/*
			
			// ratio such that f is fixed by multiply the ratio
			double ratio = cocx/sqrt(Delta_v/Klambda);
			
			NewFocal=(int) (Current_F*ratio);
			
			NewDelta_v=(((10.0*NearDist*NewFocal)/((10.0*NearDist)-NewFocal))-((FarDist*10.0*NewFocal)/((FarDist*10.0)-NewFocal))); //units in mm
			
			Newmagnify = NewFocal/((MyDist*10.0)-NewFocal);
*/

			snprintf(Alert, sizeof(Alert),"Advise focal:%d aperture:%.2f\n",BestFocal, New_aperture); 
			//(double)(sqrt((Klambda/2.0)*NewDelta_v))/(1+Newmagnify), 100.0*ratio); //  to make it and to make the ratio old_F/new_F
		  MyAlert(Alert, 2,0);
		};		
		
		C_def = (Delta_v/(2*Current_aperture*(1+magnify)));
		C_diffr = (Current_aperture*(1+magnify)/Klambda);
		C_T = sqrt((C_def*C_def)+(C_diffr*C_diffr));

		
		printf("Dist=%d, Delta_v=%f, Aperture=%f, C_def = %g, C_diffr = %g, C_T=%f\n", MyDist, Delta_v, Current_aperture,C_def, C_diffr, C_T);
		
		GoToDist2((double)(MyDist/100.0));
	};
	
	// may be 'b' for Delta _v fixe (distance sur le capteur - puis Delta_ distance dit Delta_U - 
	// est-ce intéressant car Delta_V est dépendant de la distance de focus ?
	// L'intervalle de focus semble intéressant mais dès fois c'est ridicule (50cm à 10 mètres avec focal de 55 est idiot
	// ce serait bon avec un visage à 1m sur 50-80 par exemple
	// à tester
	
		if (c_pause == 'c') {
		  int MyCoc;
  	  MyCoc=Capture_entier ("type coc in nm : 19 for 0.019");
		  cocx=(double)MyCoc/1000.0;
	
		  setfillstyle(SOLID_FILL, BLACK);
	    init_rule_dof();
	    refresh_coc ();
	    //init_Focals_HD();
	    refresh_screen() ;
	};

  if (c_pause == 'h') // set to hyperfocale
  	{
  		// 		GoToDist2((double)(MyDist/100.0));

  		printf("test Hyperfocale in mm %d \n", focus_calc_focal_H(Current_F, Current_aperture, coc));
  		GoToDist2((double)(focus_calc_focal_H(Current_F, Current_aperture, coc)/1000.0));
  	};
	
	if ((c_pause == 'd')|| (c_pause == 80))	{ // down by 1 focal
		Scroll_V(Current_F+1);
	};
	if ((c_pause == 'u')|| (c_pause == 72)) { // up by 1 focal
		Scroll_V(Current_F-1);
	};
	if ((c_pause == 'l')|| (c_pause == 77)) {Scroll_HL(20.0); };
	if ((c_pause == 'r') || (c_pause == 75)){Scroll_HR(20.0); };
 
	
	if (c_pause == 'm')  {if(N_Granularity >= 2.0) N_Granularity/=2.0;refresh_rule();}; // more ticks
	if (c_pause == 'p')  {N_Granularity*=2.0;	refresh_rule();}; // less ticks
	
	if ((c_pause == 'z')| (c_pause == '+')) {Zoom (14.0);}; // zoom in
	if ((c_pause == 'y')| (c_pause == '-')) { Zoom (-1*14.0);}; // zoom out
		
	if (c_pause == 's') { if (calc_w_diff) calc_w_diff=0; else calc_w_diff=1; refresh_coc (); refresh_screen();};
	if (c_pause == 'o') {
		int MyApper;
		MyApper=Capture_entier ("type aperture in tens: 18 for 1.8");

		Current_aperture=(double)(MyApper/10.0);
		refresh_coc ();
		refresh_screen();
	};
	
	fprintf(stderr, "\n");

// diffraction blur by approximation
// the depth of focus for diffraction limited systems ??

	fprintf(stderr, "\n");
  fprintf(stderr,"    by default we take into account in the equation diffraction+defocus then only yellow lines\n");	
	fprintf(stderr, "*** DOF : if Red lines : \n");
	fprintf(stderr, "             red lines is defocus+diffraction blur to reach camera coc;\n");
	fprintf(stderr, "             yellow lines without taking account diffraction\n") ; 
	fprintf(stderr, "|**************************************************************************|\n");
	fprintf(stderr, "|***COMMANDS to type in graphical screen and values in command terminal****|\n");
	fprintf(stderr, "|**************************************************************************|\n");
	fprintf(stderr, "|m_ou_l -more ou less sticks                                               |\n");
	fprintf(stderr, "|a-to choose near&far distance                                             |\n");
	fprintf(stderr, "|i-to refresh                                                              |\n");
	fprintf(stderr, "|g-set a distance                                                          |\n");
	fprintf(stderr, "|c-set a coc, 0,019mm by default                                           |\n");
	fprintf(stderr, "|           |           /|\\                                                |\n");
	fprintf(stderr, "|d(own) or \\|/ and u(p)  | to change the focal                             |\n");
	fprintf(stderr, "|o set aperture                                                            |\n");
	fprintf(stderr, "|z or '+' for zoom and y or '-' for unzoom                                 |\n");
	fprintf(stderr, "|left arrow '<--' or l(eft); '-->' or r(ight) change dist at left, at right|\n");
	fprintf(stderr, "|s to switch total coc is only defocus or with diffraction                 |\n");
	fprintf(stderr, "|h to set hyperfocal  distance                                             |\n");
	fprintf(stderr, "|**************************************************************************|\n");
	fprintf(stderr, "\n");
	
	c_pause=readkey();

}

return 1;

}

int main ()
{
 Init ();

}