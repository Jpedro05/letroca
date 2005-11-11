#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <allegro.h>

/* Constantes */

#define MAX_X 640
#define MAX_Y 480
#define CORES 16
#define NUM_LET 7
#define LETRAS 26

/* Globais */
int som = 1;
int janela = 0;
volatile int ctrl_vel;
volatile int tim_tempo;

/* Estruturas */

struct maior
{
	char palavra[16];
	int ligado[16];
};

struct subgrupos
{
	/*int indice;*/
	int numletras;
	char palavra[16];
	int ligado[16];
};

/* Escopos */

int inicializa ( void );
void finaliza ( void );

void jogo ( void );

void intro ( BITMAP *buffer, BITMAP *fundo, BITMAP **Letras );
void menu ( BITMAP *buffer, BITMAP *fundo, BITMAP **Letras );
void letroca (BITMAP *buffer, BITMAP *fundo, BITMAP **Letras, int tempo);
void sobre ( BITMAP *buffer );

int subgrupoSort (const void *ant, const void *post);
void embaralhaFunc ( struct maior *palavraMaior, int tam );

void inc_ctrl_vel (void);
void dec_tim_tempo (void);

/* Main */

int main (int argc, char **argv)
{
	int i;
	if ( argc > 1 )
	{
		for ( i = 1 ; i < argc ; i++ )
		{
			if ( !strcmpi(argv[i],"-janela") )
			{
				janela = 1;
			}
		}
	}
	
	if ( !inicializa() )
	{
		finaliza();
		return 1;
	}
	
	jogo();
	
	finaliza();
	
	return 0;
}
END_OF_MAIN();

/* Funções de Início e Fim da biblioteca */

int inicializa (void)
{
	allegro_init();
	
	install_mouse();
	
	install_keyboard();
	
	install_timer();
	
	if (install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL) < 0)
	{
		som = 0;
	}

	if ( janela )
	{
		if ( desktop_color_depth() < CORES )
		{
			set_color_depth(desktop_color_depth());
		}
		else
		{
			set_color_depth(CORES);
		}
		if ( set_gfx_mode( GFX_AUTODETECT_WINDOWED, MAX_X, MAX_Y, 0, 0 ) < 0 )
		{
			return FALSE;
		}
	}
	
	else
	{
		set_color_depth(CORES);
		if ( set_gfx_mode(GFX_AUTODETECT, MAX_X, MAX_Y, 0, 0) < 0 )
		{
			return FALSE;
		}
	}
	
	return TRUE;
}


void finaliza (void)
{
	allegro_exit();
}

/* Início das funções do jogo */

void jogo (void)
{
	int i;
	BITMAP *buffer, *fundo, *letras[LETRAS];
	char nome[5];
	
	/* Carrega todas as letras na memória */
	for ( i = 0 ; i < LETRAS ; i++ )
	{
		sprintf(nome,"%c.bmp",'A'+i);
		letras[i] = load_bitmap(nome,NULL);
	}
	
	buffer = create_bitmap(MAX_X, MAX_Y);
	fundo = load_bitmap("back.bmp",NULL);
	if ( !buffer )
	{
		return;
	}
	
	clear_to_color(screen,makecol(0,0,0));
	
	ctrl_vel = 0;
	LOCK_VARIABLE(ctrl_vel);
	LOCK_FUNCTION(inc_ctrl_vel);
	install_int(inc_ctrl_vel, 1000 / 5);
	
	intro(buffer, fundo, letras);
	
	clear_to_color(screen,makecol(0,0,0));
	
	menu(buffer, fundo, letras);
}

void intro (BITMAP *buffer, BITMAP *fundo, BITMAP **Letras)
{
	int fade,i, j,k;
	BITMAP *letras[NUM_LET];
	char nome[20];
	
	/* Carrega os sprites das palavras na memória 
	for ( i = 0 ; i < NUM_LET ; i++ )
	{
		sprintf(nome,"letroca%d.bmp",i+1);
		letras[i] = load_bitmap(nome,NULL);
		if (!letras[i])
		{
			printf("Falhou\n");
			return;
		}
	}*/
	
	/* carrega as letras */
	letras[0] = Letras['L'-'A'];
	letras[1] = Letras['E'-'A'];
	letras[2] = Letras['T'-'A'];
	letras[3] = Letras['R'-'A'];
	letras[4] = Letras['O'-'A'];
	letras[5] = Letras['C'-'A'];
	letras[6] = Letras['A'-'A'];
	
	j = 0;
	
	/* Fade é a variável que controla se a cor está ficando mais clara ou mais escura
	i é a variável que contém a cor atual */
	fade = -8;
	i = 255;
	
	/* Faz o fundo do texto ficar transparente e limpa o buffer do teclado */
	text_mode(-1);
	clear_keybuf();
	
	ctrl_vel = 0;
	
	while ( 1 )
	{
		if ( ctrl_vel > 0 )
		{
			/*clear_to_color(buffer,makecol(0,128,0));
			Copia o fundo para o buffer*/
			blit(fundo, buffer, 0, 0, 0, 0, fundo->w, fundo->h);
			textout_centre(buffer, font, "Pressione ENTER para continuar", buffer->w / 2,
			buffer->h - 70, makecol(0,i,0));
			i += fade;
			if ( i < 0 || i > 255)
			{
				fade *= -1;
				/* Se i for menor do que 0, então i = 0, se não, i = 255 */
				i = i<0 ? 0 : 255;
			}
			
			/* Desenha a letra se o número dela não for igual a j */
			for ( k = 0 ; k < NUM_LET ; k++ )
			{
				if ( k != j )
				{
					draw_sprite(buffer,letras[k],150+50*k,50);
				}
			}
			
			/*draw_sprite(buffer,letras[j],(buffer->w/2)-(letras[j]->w/2),30);*/
			j = ( j + 1 ) % (NUM_LET+5);
			
			blit(buffer, screen, 0, 0, 0, 0, buffer->w, buffer->h);
			if ( keypressed() )
			{
				if ( ( readkey() >> 8 ) == KEY_ENTER )
				{
					break;
				}
			}
			ctrl_vel--;
		}
	}
}

void menu (BITMAP *buffer, BITMAP *fundo, BITMAP **Letras)
{
	BITMAP *sairb, *menub, *sobreb, *jogob, *jogotempob, *menul[4];
	int sizex, sizey;

	menul[0] = Letras['M'-'A'];
	menul[1] = Letras['E'-'A'];
	menul[2] = Letras['N'-'A'];
	menul[3] = Letras['U'-'A'];
	
	/* carrega os botões na memória */
	menub = load_bitmap("menu.bmp",NULL);
	sairb  = load_bitmap("sair.bmp",NULL);
	sobreb = load_bitmap("sobre.bmp",NULL);
	jogob = load_bitmap("jogo.bmp",NULL);
	jogotempob = load_bitmap("jogotempo.bmp",NULL);
	
	sizex = sairb->w;
	sizey = sairb->h;
	
	while ( 1 )
	{
		show_mouse(NULL);
		blit(fundo, buffer, 0, 0, 0, 0, fundo->w, fundo->h);
		
		/* Desenha "Menu" */
		draw_sprite(buffer,menul[0],buffer->w/2-(2*50),60);
		draw_sprite(buffer,menul[1],buffer->w/2-50,60);
		draw_sprite(buffer,menul[2],buffer->w/2,60);
		draw_sprite(buffer,menul[3],buffer->w/2+50,60);
		
		/* desenha os botões */
		draw_sprite(buffer,jogob,70,161);
		draw_sprite(buffer,jogotempob,350,237);
		draw_sprite(buffer,sobreb,70,314);
		draw_sprite(buffer,sairb,350,392);
		
		/* se o botão direito for apertado */
		if ( ( mouse_b % 2 ) == 1 )
		{
			/* se está na coluna da esquerda */
			if ( mouse_x >= 70 && mouse_x <= 70+sizex ) 
			{
				/* se está sobre o "novo jogo" */
				if ( mouse_y >= 161 && mouse_y <= 161+sizey )
				{
					letroca (buffer, fundo, Letras, 0);
				}
				/* se está sobre o "sobre" */
				else if ( mouse_y >= 314 && mouse_y <= 314+sizey )
				{
					sobre(buffer);
				}
			}
			
			/* se está na coluna da direita */
			else if ( mouse_x >= 350 && mouse_x <= 350+sizex ) 
			{
				/* se está sobre o "jogo com tempo" */
				if ( mouse_y >= 237 && mouse_y <= 237+sizey )
				{
					letroca (buffer, fundo, Letras, 1);
				}
				/* se está sobre o "sair" */
				else if ( mouse_y >= 392 && mouse_y <= 392+sizey )
				{
					break;
				}
			}
		}
		
		show_mouse(buffer);
		
		blit(buffer, screen, 0, 0, 0, 0, buffer->w, buffer->h);
	}
}

void letroca (BITMAP *buffer, BITMAP *fundo, BITMAP **Letras, int tempo)
{
	struct maior palavraMaior;
	struct subgrupos *palavras;
	int i, j, tam, quantas;
	
	srand(time(NULL));
	
	/* Implementar função para: Sortear palavra do dicionário e carregar na memória */
	sprintf(palavraMaior.palavra,"PARALELO");
	tam = 8;
	
	i = 0;
	for ( ; i < tam ; i++ )
	{
		palavraMaior.ligado[i] = 1;
	}
	palavraMaior.ligado[i] = 0;
	
	quantas = 6;
	/* 
	pipa 
	lepra
	pedra
	para
	parede
	paralelo
	pelo
	ele
	ela
	ralar
	ler
	*/
	
	palavras = (struct subgrupos *) malloc ( quantas * sizeof(struct subgrupos));
	if (!palavras) exit (1);
	
	for ( i = 0 ; i < quantas ; i++ )
	{
		for ( j = 0 ; j < 15 ; j++ )
		{
			palavras[i].ligado[j] = 0;
		}
	}
	
	sprintf(palavras[0].palavra , "Lepra");
	palavras[0].numletras = 5;
	sprintf(palavras[1].palavra , "Para");
	palavras[1].numletras = 4;
	sprintf(palavras[2].palavra , "Paralelo");
	palavras[2].numletras = 8;
	sprintf(palavras[3].palavra , "Pelo");
	palavras[3].numletras = 4;
	sprintf(palavras[4].palavra , "Ele");
	palavras[4].numletras = 3; 
	sprintf(palavras[5].palavra , "Ela");
	palavras[5].numletras = 3;
	sprintf(palavras[6].palavra , "Ler");
	palavras[6].numletras = 3;
	
	qsort(palavras, quantas, sizeof(struct subgrupos),subgrupoSort);
	
	embaralhaFunc(&palavraMaior,tam);
	
	if ( tempo )
	{
			tim_tempo = quantas * 20;
			tim_tempo = (tim_tempo<180) ? 180 : tim_tempo;
			LOCK_VARIABLE(tim_tempo);
			LOCK_FUNCTION(dec_tim_tempo);
			install_int(dec_tim_tempo, 1000);
	}
	
	/*while ( 1 )
	{*/
		blit(fundo, buffer, 0, 0, 0, 0, fundo->w, fundo->h);
		for ( i = 0 ; i < tam ; i++ )
		{
			/* Desenha a palavra embaralhada */
			if ( palavraMaior.ligado[i] )
			{
				draw_sprite(buffer , Letras[ (palavraMaior.palavra[i] - 'A') ] , 15+((Letras[0]->w+2)*i) , buffer->h-65);
			}
			else
			{
				rect(buffer,15+((Letras[0]->w+2)*i),buffer->h-65,15+((Letras[0]->w+2)*(i+1)),buffer->h-15,makecol(255,255,255));
			}
			/* Desenha a palavra selecionada */
			if ( !palavraMaior.ligado[i] )
			{
				draw_sprite(buffer , Letras[ (palavraMaior.palavra[i] - 'A') ] , 15+((Letras[0]->w+2)*i) , buffer->h-65);
			}
			else
			{
				rect(buffer,15+((Letras[0]->w+2)*i),buffer->h-115,15+((Letras[0]->w+2)*(i+1)),buffer->h-70,makecol(255,255,255));
			}
		}
		blit(buffer, screen, 0, 0, 0, 0, buffer->w, buffer->h);
		getchar();
	/*}*/
}

int subgrupoSort (const void *ant, const void *post)
{
	struct subgrupos *a, *b;
	a = (struct subgrupos *) ant;
	b = (struct subgrupos *) post;
	
	if ( a->numletras < b->numletras )
	{
		return -1;
	}
	else if ( a->numletras > b->numletras )
	{
		return 1;
	}
	else
	{
		/* MUDAR NO DIA QUE FOR APRESENTAR */
		return strcasecmp(a->palavra,b->palavra);
	}
}

void embaralhaFunc ( struct maior *palavraMaior, int tam )
{
	char tempP[16], temp;
	int e1, e2, tempL, i;
	
	/* Embaralha a palavra maior */
	strcpy(tempP,palavraMaior->palavra);
	do
	{
		for ( i = 0 ; i < tam ; i++ )
		{
			e1 = rand() % tam;
			do
			{
				e2 = rand() % tam;
			} while ( e2 == e1 );
			
			temp = tempP[e1];
			tempP[e1] = tempP[e2];
			tempP[e2] = temp;
			
			tempL = palavraMaior->ligado[e1];
			palavraMaior->ligado[e1] = palavraMaior->ligado[e2];
			palavraMaior->ligado[e2] = tempL;
		}
	} while ( !strcasecmp(tempP,palavraMaior->palavra) );
	strcpy(palavraMaior->palavra, tempP);
}

void sobre (BITMAP *buffer)
{
	int sizeX, sizeY, janelaX, janelaY, bordaJanX, bordaJanY, botaoX, botaoY, bordaBotX, bordaBotY;
	
	sizeX = buffer->w/2;
	sizeY = buffer->h/2;
	
	/* Janela de tamanho 280x200 
	   desenha um retângulo desse tamanho, 
	   depois colore por dentro, 
	   deixando uma borda de 1px
	*/
	bordaJanX = 280 / 2;
	janelaX = bordaJanX - 1;
	bordaJanY = 200 / 2;
	janelaY = bordaJanY - 1;
	
	rect(buffer, sizeX-bordaJanX, sizeY-bordaJanY, sizeX+bordaJanX, sizeY+bordaJanY, 
	makecol(255,255,255));
	rectfill(buffer, sizeX-janelaX, sizeY-janelaY, sizeX+janelaX, sizeY+janelaY, 
	makecol(0,0,0));
	
	/* Botão de 40x20 
	   Idem acima.
	*/
	bordaBotX = 40 / 2;
	botaoX = bordaBotX - 1;
	bordaBotY = 20;
	botaoY = bordaBotY - 1;
	
	rect(buffer, sizeX-bordaBotX, sizeY+60, sizeX+bordaBotX, sizeY+bordaBotY+60, 
	makecol(255,255,255));
	rectfill(buffer, sizeX-botaoX, sizeY+61, sizeX+botaoX, sizeY+botaoY+60, makecol(0,0,0));
	
	mouse_b = 0;
	
	while ( ( mouse_b != 1 ) || (mouse_x < sizeX-bordaBotX || mouse_x > sizeX+bordaBotX) || (mouse_y < sizeY+60 || mouse_y > sizeY+bordaBotY+60) )
	{
		show_mouse(buffer);
		blit(buffer, screen, 0, 0, 0, 0, buffer->w, buffer->h);
		show_mouse(NULL);
	}
}

void inc_ctrl_vel (void)
{
	ctrl_vel++;
}
END_OF_FUNCTION(inc_ctrl_vel);

void dec_tim_tempo (void)
{
	tim_tempo--;
}
END_OF_FUNCTION(dec_tim_tempo);
