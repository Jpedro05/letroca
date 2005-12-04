/*
	Programa:  Letroca.c
	Aluno:		Cinila de Jesus Machado,
				Renan Moreira Lobo
				Renato dos Santos Cerqueira
	Data:      07/11/05 - 04/12/05
	Assunto:	Este é o jogo Letroca, feito como trabalho de fim de período de
				2005.2 da turma de comp1.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
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
int score = 0;
volatile int ctrl_vel;
volatile int tim_tempo;

/* Estruturas */

typedef struct _maior
{
	char palavra[16];
	int ligado[16];
} MAIOR;

typedef struct _subgrupos
{
	int numletras;
	char palavra[16];
	int ligado;
} SUBGRUPOS;

typedef struct _jogador
{
	char nome[40];
	int recorde;
} JOGADOR; 

/* Escopos */

int inicializa ( void );
void finaliza ( void );

void jogo ( void );

void intro ( BITMAP *buffer, BITMAP *fundo, BITMAP **Letras );
void menu (BITMAP *buffer, BITMAP *fundo, BITMAP **Letras, JOGADOR *recordistas, SAMPLE *clique, SAMPLE *erro);
void letroca (BITMAP *buffer, BITMAP *fundo, BITMAP **Letras, int tempo, SAMPLE *clique, SAMPLE *erro);
void sobre ( BITMAP *buffer , SAMPLE *clique);
void palavraNaoEncontrada (BITMAP *buffer, int tempo, SAMPLE *clique);
void palavraRepetida (BITMAP *buffer, int tempo, SAMPLE *clique);
int desejaSair (BITMAP *buffer, int tempo, SAMPLE *clique);
int gOver (BITMAP *buffer, int proxima, SAMPLE *clique);
void novoRecord ( BITMAP *buffer, JOGADOR *recordistas, SAMPLE *clique );
void exibeRecord ( BITMAP *buffer, JOGADOR *recordistas, SAMPLE *clique );

void sorteia ( MAIOR *palavraMaior, SUBGRUPOS *palavras, int *quantas );
void leRecords ( JOGADOR *recordistas );
void guardaRecords ( JOGADOR *recordistas );
int subgrupoSort (const void *ant, const void *post);
void embaralhaFunc ( MAIOR *palavraMaior, int tam );

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
			if ( !strcasecmp(argv[i],"-janela") )
			{
				janela = 1;
			}
			if ( !strcasecmp(argv[i],"-semsom") )
			{
				som = 0;
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
	set_uformat(U_ASCII);
	
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
	JOGADOR recordistas[11];
	MIDI	*musica;
	SAMPLE	*erro, *clique;
	char nome[5];
	
	leRecords(recordistas);
	/* Carrega todas as letras na memória */
	for ( i = 0 ; i < LETRAS ; i++ )
	{
		sprintf(nome,"./imagens/%c.bmp",'A'+i);
		letras[i] = load_bitmap(nome,NULL);
		if ( !letras[i] )
		{
			perror("load_bitmap");
			exit(1);
		}
	}
	
	buffer = create_bitmap(MAX_X, MAX_Y);
	if ( !buffer )
	{
		perror("create_bitmap");
		exit(1);
	}
	
	fundo = load_bitmap("./imagens/back.bmp",NULL);
	if ( !fundo )
	{
		perror("load_bitmap");
		exit(1);
	}
	
	if ( som )
	{
		musica = load_midi("./sounds/moonlight.mid");
		if (!musica)
		{
			perror("load_midi");
			exit(1);
		}
		erro = load_sample("./sounds/erro.wav");
		if (!erro)
		{
			perror("load_sample");
			exit(1);
		}
		clique = load_sample("./sounds/clique.wav");
		if (!clique)
		{
			perror("load_sample");
			exit(1);
		}
		play_midi(musica, TRUE);
	}
	
	clear_to_color(screen,makecol(0,0,0));
	
	ctrl_vel = 0;
	LOCK_VARIABLE(ctrl_vel);
	LOCK_FUNCTION(inc_ctrl_vel);
	install_int(inc_ctrl_vel, 1000 / 5);
	
	intro(buffer, fundo, letras);
	
	clear_to_color(screen,makecol(0,0,0));
	
	menu(buffer, fundo, letras, recordistas, clique, erro);
	
	destroy_bitmap(buffer);
	destroy_bitmap(fundo);
	for ( i = 0 ; i < LETRAS ; i++ )
	{
		destroy_bitmap(letras[i]);
	}
	
	if ( som )
	{
		stop_midi();
		destroy_midi(musica);
		destroy_sample(erro);
		destroy_sample(clique);
	}
	
	guardaRecords(recordistas);
}

void intro (BITMAP *buffer, BITMAP *fundo, BITMAP **Letras)
{
	int fade, i, j, k, pressed;
	BITMAP *letras[NUM_LET];
	
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
				pressed = readkey();
				if ( ( pressed >> 8 ) == KEY_ENTER || ( pressed >> 8 ) == KEY_ENTER_PAD )
				{
					break;
				}
			}
			ctrl_vel--;
		}
	}
}

void menu (BITMAP *buffer, BITMAP *fundo, BITMAP **Letras, JOGADOR *recordistas, SAMPLE *clique, SAMPLE *erro)
{
	BITMAP *sairb, *sobreb, *jogob, *jogotempob, *recordesb, *menul[4];
	int sizex, sizey, pressed;

	menul[0] = Letras['M'-'A'];
	menul[1] = Letras['E'-'A'];
	menul[2] = Letras['N'-'A'];
	menul[3] = Letras['U'-'A'];
	
	/* carrega os botões na memória */
	sairb  = load_bitmap("./imagens/sair.bmp",NULL);
	if (!sairb) { perror("load_bitmap"); exit(1); }
	sobreb = load_bitmap("./imagens/sobre.bmp",NULL);
	if (!sobreb) { perror("load_bitmap"); exit(1); }
	jogob = load_bitmap("./imagens/jogo.bmp",NULL);
	if (!jogob) { perror("load_bitmap"); exit(1); }
	jogotempob = load_bitmap("./imagens/jogotempo.bmp",NULL);
	if (!jogotempob) { perror("load_bitmap"); exit(1); }
	recordesb = load_bitmap("./imagens/recordes.bmp",NULL);
	if (!recordesb) { perror("load_bitmap"); exit(1); }
	
	sizex = sairb->w;
	sizey = sairb->h;
	
	pressed = 0;
	
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
		draw_sprite(buffer,jogob,32,161);
		draw_sprite(buffer,jogotempob,sizex+(32*2),161);
		draw_sprite(buffer,sobreb,32,280);
		draw_sprite(buffer,recordesb,sizex+(32*2),280);
		draw_sprite(buffer,sairb,((MAX_X/2)-(sizex/2)),400);
		
		/* se o botão direito for apertado */
		if ( ( mouse_b % 2 ) == 1 )
		{
			/* se está na coluna da esquerda */
			if ( mouse_x >= 32 && mouse_x <= 32+sizex ) 
			{
				/* se está sobre o "novo jogo" */
				if ( mouse_y >= 161 && mouse_y <= 161+sizey )
				{
					if ( som )
					{
						play_sample(clique, 255, 128, 1000, FALSE);
					}
					letroca (buffer, fundo, Letras, 0, clique, erro);
					if ( score >= recordistas[9].recorde )
					{
						novoRecord(buffer, recordistas, clique);
					}
					mouse_b = 0;
					score = 0;
				}
				/* se está sobre o "sobre" */
				else if ( mouse_y >= 280 && mouse_y <= 280+sizey )
				{
					if ( som )
					{
						play_sample(clique, 255, 128, 1000, FALSE);
					}
					sobre(buffer, clique);
					mouse_b=0;
				}
			}
			
			/* se está na coluna da direita */
			else if ( mouse_x >= (32*2)+sizex && mouse_x <= (32*2)+(sizex*2) ) 
			{
				/* se está sobre o "jogo com tempo" */
				if ( mouse_y >= 161 && mouse_y <= 161+sizey )
				{
					if ( som )
					{
						play_sample(clique, 255, 128, 1000, FALSE);
					}
					letroca (buffer, fundo, Letras, 1, clique, erro);
					if ( score >= recordistas[9].recorde )
					{
						novoRecord(buffer, recordistas, clique);
					}

					mouse_b = 0;
					score = 0;
				}
				/* se está sobre o "recordes" */
				else if ( mouse_y > 280 && mouse_y <= 280+sizey )
				{
					if ( som )
					{
						play_sample(clique, 255, 128, 1000, FALSE);
					}
					exibeRecord(buffer, recordistas, clique);
				}
			}
			
			/* se está sobre o "sair" */
			if ( mouse_x >= ((MAX_X/2)-(sizex/2)) && mouse_x <= ((MAX_X/2)-(sizex/2))+sizex )
			{
				if ( mouse_y >= 400 && mouse_y <= 400+sizey )
				{
					if ( som )
					{
						play_sample(clique, 255, 128, 1000, FALSE);
					}
					break;
				}
			}
		}
		/* Se alguma tecla for pressionada */
		if ( keypressed() )
		{
			pressed = readkey();
			if ( (pressed >> 8) == KEY_ESC )
			{
				break;
			}
		}
		
		show_mouse(buffer);
		
		blit(buffer, screen, 0, 0, 0, 0, buffer->w, buffer->h);
	}
	destroy_bitmap(sairb);
	destroy_bitmap(jogob);
	destroy_bitmap(jogotempob);
	destroy_bitmap(sobreb);
	destroy_bitmap(recordesb);
}

void letroca (BITMAP *buffer, BITMAP *fundo, BITMAP **Letras, int tempo, SAMPLE *clique, SAMPLE *erro)
{
	MAIOR palavraMaior, palavraTemp;
	SUBGRUPOS palavras[20];
	int i, j, tam, quantas, pressed, usadas, descobertas, desistir, achou, redraw, proxima,
	bordaBotX, bordaBotY, botaoX, botaoY, sizeX, sizeY;
	char letra;
	
	srand(time(NULL));
	
	/* Sortear palavras do dicionário e carregar na memória */
	sorteia(&palavraMaior, palavras, &quantas);
	
	tam = strlen(palavraMaior.palavra);
	
	i = 0;
	for ( ; i < tam ; i++ )
	{
		palavraMaior.ligado[i] = 1;
		palavraTemp.ligado[i] = 0;
	}
	palavraMaior.ligado[i] = 0;
	palavraTemp.ligado[i] = 0;

	for ( i = 0 ; i < quantas ; i++ )
	{
		palavras[i].ligado = 0;
	}
	
	qsort(palavras, quantas, sizeof(SUBGRUPOS),subgrupoSort);
	
	embaralhaFunc(&palavraMaior,tam);
	
	clear_keybuf();
	usadas = descobertas = desistir = 0;
	
	if ( tempo )
	{
			tim_tempo = quantas * 20;
			tim_tempo = (tim_tempo<180) ? 180 : tim_tempo;
			LOCK_VARIABLE(tim_tempo);
			LOCK_FUNCTION(dec_tim_tempo);
			install_int(dec_tim_tempo, 1000);
	}
	redraw = 0;
	
	sizeX = MAX_X-58;
	sizeY = MAX_Y/2;
	
	/* Botão de 40x20 
	   Idem acima.
	*/
	bordaBotX = 85 / 2;
	botaoX = bordaBotX - 1;
	bordaBotY = 20;
	botaoY = bordaBotY - 1;
	
	while ( 1 )
	{
		if ( desistir ) 
		{ 
			redraw = 1; 
		}
		show_mouse(NULL);
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
			if ( palavraTemp.ligado[i] )
			{
				draw_sprite(buffer , Letras[ (palavraTemp.palavra[i] - 'A') ] , 15+((Letras[0]->w+2)*i) , buffer->h-115);
			}
			else
			{
				rect(buffer,15+((Letras[0]->w+2)*i),buffer->h-115,15+((Letras[0]->w+2)*(i+1)),buffer->h-70,makecol(255,255,255));
			}
		}
		
		/* Desenha a lista de palavras do lado esquerdo*/
		for ( i = 0 ; i < quantas && i < 10 ; i++ )
		{
			if ( palavras[i].ligado )
			{
				for ( j = 0 ; palavras[i].palavra[j] ; j++ )
				{
					stretch_sprite(buffer, Letras[ (palavras[i].palavra[j] - 'A') ] , (15+(25+2)*j) , (15+(30*i)), 25, 25);
				}
			}
			else
			{
				for ( j = 0 ; palavras[i].palavra[j] ; j++ )
				{
					rect(buffer, 15+((25+2)*j), 15+(30*i), 15+((25+2)*(j+1)), 15+(30*i)+25, makecol(255,255,255));
				}
			}
		}
		
		/* Desenha a lista de palavras do lado direito */
		if ( quantas > 10 )
		{
			for ( i = 10 ; i < quantas ; i++ )
			{
				if ( palavras[i].ligado )
				{
					for ( j = 0 ; palavras[i].palavra[j] ; j++ )
					{
						stretch_sprite(buffer, Letras[ (palavras[i].palavra[j] - 'A') ] , (240+(25+2)*j) , (15+(30*(i-10))), 25, 25);
					}
				}
				else
				{
					for ( j = 0 ; palavras[i].palavra[j] ; j++ )
					{
						rect(buffer, 240+((25+2)*j), 15+(30*(i-10)), 240+((25+2)*(j+1)), 15+(30*(i-10))+25, makecol(255,255,255));
					}
				}
			}
		}
		
		/* Botões */
		rect(buffer, sizeX-bordaBotX, sizeY+35, sizeX+bordaBotX, sizeY+bordaBotY+35, 
		makecol(255,255,255));
		rectfill(buffer, sizeX-botaoX, sizeY+36, sizeX+botaoX, sizeY+botaoY+35, 
		makecol(0,0,0));
		
		rect(buffer, sizeX-bordaBotX, sizeY+75, sizeX+bordaBotX, sizeY+bordaBotY+75, 
		makecol(255,255,255));
		rectfill(buffer, sizeX-botaoX, sizeY+76, sizeX+botaoX, sizeY+botaoY+75, 
		makecol(0,0,0));
		
		rect(buffer, sizeX-bordaBotX, sizeY+115, sizeX+bordaBotX, sizeY+bordaBotY+115, 
		makecol(255,255,255));
		rectfill(buffer, sizeX-botaoX, sizeY+116, sizeX+botaoX, sizeY+botaoY+115, 
		makecol(0,0,0));
		
		rect(buffer, sizeX-bordaBotX, sizeY+155, sizeX+bordaBotX, sizeY+bordaBotY+155, 
		makecol(255,255,255));
		rectfill(buffer, sizeX-botaoX, sizeY+156, sizeX+botaoX, sizeY+botaoY+155, 
		makecol(0,0,0));
		
		/* Escrita */
		text_mode(-1);
		
		textprintf_centre(buffer, font, sizeX, sizeY+35+(bordaBotY/2), makecol(255,255,255),
		"Desistir");
		
		textprintf_centre(buffer, font, sizeX, sizeY+75+(bordaBotY/2), makecol(255,255,255),
		"Embaralhar");
		
		textprintf_centre(buffer, font, sizeX, sizeY+115+(bordaBotY/2), makecol(255,255,255),
		"Limpar");
		
		textprintf_centre(buffer, font, sizeX, sizeY+155+(bordaBotY/2), makecol(255,255,255),
		"Ok");
		
		/* Caixa de Score */
		rectfill(buffer, buffer->w-100, 15, buffer->w-15, 50, makecol(0,0,0));
		textprintf_centre(buffer, font, buffer->w-57, 30, makecol(255,255,255),
		"%05d", score );
		
		/* Caixa de tempo */
		if ( tempo )
		{
			rectfill(buffer, buffer->w-100, 65, buffer->w-15, 100, makecol(0,0,0));
			textprintf_centre(buffer, font, buffer->w-57, 80, makecol(255,255,255),
			"%02d:%02d", tim_tempo/60, tim_tempo%60 );
		}
		
		if ( keypressed() && !desistir ) 
		{ 
			if ( som )
			{
				play_sample(clique, 255, 128, 1000, FALSE);
			}
			pressed = readkey();
			
			letra = (pressed & 0xFF);
			if ( (pressed >> 8) == KEY_BACKSPACE && usadas > 0)
			{
				usadas--;
				palavraMaior.ligado[palavraTemp.ligado[usadas]-1] = 1;
				palavraTemp.ligado[usadas] = 0;
			}
			if ( (pressed >> 8) == KEY_ENTER || (pressed >> 8) == KEY_ENTER_PAD )
			{
				/* Vê se tem mais de três letras digitadas */
				if ( usadas >= 3 )
				{
					achou = 0;
					/* Varre todas as palavras com número de letra menor ou igual ao de
					   letras digitadas */
					for ( i = 0 ; palavras[i].numletras <= usadas && i < quantas ; i++ )
					{
						/* Verifica letra a letra quais são iguais, quando encontra 
						uma diferente, sai do For */
						for ( j = 0 ; j < usadas ; j++ )
						{
							if ( palavraTemp.palavra[j] != palavras[i].palavra[j] )
							{
								break;
							}
						}
						/* Se o número de letras iguais for igual ao número de letras
						   da palavra que está buscando e se for igual ao de letras 
						   digitadas */
						if ( j == palavras[i].numletras && j == usadas )
						{
							/* Caso a palavra ainda não esteja aparecendo */
							if ( !palavras[i].ligado )
							{
								palavras[i].ligado = 1;
								score += 100 * j;
								/* Varre as letras digitadas desligando-as e religando a 
								   palavra embaralhada */
								for ( j = 0 ; j < usadas ; j++ )
								{
									palavraMaior.ligado[ palavraTemp.ligado[j]-1 ] = 1;
									palavraTemp.ligado[j] = 0;
								}
								usadas = 0;
								descobertas++;
								achou = 1;
							}
							else
							{
								if ( som )
								{
									play_sample(erro, 255, 128, 1000, FALSE);
								}
								palavraRepetida(buffer,tempo,clique);
								pressed = 0;
								achou = 1;
								for ( i = 0 ; i < usadas ; i++ )
								{
									palavraMaior.ligado[palavraTemp.ligado[i]-1] = 1;
									palavraTemp.ligado[i] = 0;
								}
								usadas = 0;
								break;
							}
						}
					}
					if ( !achou )
					{
						if ( som )
						{
							play_sample(erro, 255, 128, 1000, FALSE);
						}
						palavraNaoEncontrada(buffer,tempo,clique);
						for ( i = 0 ; i < usadas ; i++ )
						{
							palavraMaior.ligado[palavraTemp.ligado[i]-1] = 1;
							palavraTemp.ligado[i] = 0;
						}
						usadas = 0;
						pressed = 0;
					}
				}
			}
			if ( (pressed >> 8) == KEY_ESC )
			{
				desistir = desejaSair(buffer,tempo,clique);
			}
			if ( (pressed >> 8) == KEY_SPACE )
			{
				for ( i = 0 ; i < usadas ; i++ )
				{
					palavraMaior.ligado[ palavraTemp.ligado[i]-1 ] = 1;
					palavraTemp.ligado[i] = 0;
				}
				usadas = 0;
				embaralhaFunc(&palavraMaior,tam);
			}
			for ( i = 0 ; i < tam ; i++ )
			{
				if ( toupper(letra) == palavraMaior.palavra[i] && palavraMaior.ligado[i] )
				{
					palavraMaior.ligado[i] = 0;
					palavraTemp.ligado[usadas] = i+1;
					palavraTemp.palavra[usadas] = toupper(letra);
					usadas++;
					break;
				}
			} 
		}
		
		/* mouse */
		if ( mouse_b == 1 )
		{
			if ( som )
			{
				play_sample(clique, 255, 128, 1000, FALSE);
			}
			if ( mouse_x > 15 && mouse_x < 15+((Letras[0]->w+2)*(tam)))
			{
				if ( ( mouse_y > buffer->h-65 && mouse_y < buffer->h-15 ) )
				{
					i = (mouse_x - 15)/(Letras[0]->w+2);
					if ( palavraMaior.ligado[i] )
					{
						palavraMaior.ligado[i] = 0;
						palavraTemp.ligado[usadas] = i+1;
						palavraTemp.palavra[usadas] = palavraMaior.palavra[i];
						usadas++;
					}
				}
			}
			if ( mouse_y > buffer->h-115 && mouse_y < buffer->h-65 && usadas)
			{
				if ( mouse_x > 15+((Letras[0]->w+2)*(usadas-1)) && mouse_x < 15+((Letras[0]->w+2)*(usadas)) )
				{
					usadas--;
					palavraMaior.ligado[palavraTemp.ligado[usadas]-1] = 1;
					palavraTemp.ligado[usadas] = 0;

				}
			}
			
			if ( mouse_x >= sizeX-bordaBotX && mouse_x <= sizeX+bordaBotX )
			{
				/* Desistir */
				if ( mouse_y >= sizeY+35 && mouse_y <= sizeY+bordaBotY+35 )
				{
					desistir = desejaSair(buffer,tempo,clique);
				}
				/* Embaralhar */
				else if ( mouse_y >= sizeY+75 && mouse_y <= sizeY+bordaBotY+75 )
				{
					for ( i = 0 ; i < usadas ; i++ )
					{
						palavraMaior.ligado[ palavraTemp.ligado[i]-1 ] = 1;
						palavraTemp.ligado[i] = 0;
					}
					usadas = 0;
					embaralhaFunc(&palavraMaior,tam);
				}
				/* Limpar */
				else if ( mouse_y >= sizeY+115 && mouse_y <= sizeY+bordaBotY+115 )
				{
					for ( i = 0 ; i < usadas ; i++ )
					{
						palavraMaior.ligado[palavraTemp.ligado[i]-1] = 1;
						palavraTemp.ligado[i] = 0;
					}
					usadas = 0;
				}
				/* Ok */
				else if ( mouse_y >= sizeY+155 && mouse_y <= sizeY+bordaBotY+155 )
				{
					/* Vê se tem mais de três letras digitadas */
					if ( usadas >= 3 )
					{
						achou = 0;
						/* Varre todas as palavras com número de letra menor ou igual ao de
						   letras digitadas */
						for ( i = 0 ; palavras[i].numletras <= usadas && i < quantas ; i++ )
						{
							/* Verifica letra a letra quais são iguais, quando encontra 
							uma diferente, sai do For */
							for ( j = 0 ; j < usadas ; j++ )
							{
								if ( palavraTemp.palavra[j] != palavras[i].palavra[j] )
								{
									break;
								}
							}
							/* Se o número de letras iguais for igual ao número de letras
							   da palavra que está buscando e se for igual ao de letras 
							   digitadas */
							if ( j == palavras[i].numletras && j == usadas )
							{
								/* Caso a palavra ainda não esteja aparecendo */
								if ( !palavras[i].ligado )
								{
									palavras[i].ligado = 1;
									score += 100 * j;
									/* Varre as letras digitadas desligando-as e religando a 
									   palavra embaralhada */
									for ( j = 0 ; j < usadas ; j++ )
									{
										palavraMaior.ligado[ palavraTemp.ligado[j]-1 ] = 1;
										palavraTemp.ligado[j] = 0;
									}
									usadas = 0;
									descobertas++;
									achou = 1;
								}
								else
								{
									if ( som )
									{
										play_sample(erro, 255, 128, 1000, FALSE);
									}
									palavraRepetida(buffer,tempo,clique);
									pressed = 0;
									achou = 1;
									break;
								}
							}
						}
						if ( !achou )
						{
							if ( som )
							{
								play_sample(erro, 255, 128, 1000, FALSE);
							}
							palavraNaoEncontrada(buffer,tempo,clique);
							pressed = 0;
						}
					}
				}
			}
		}
		
		if ( desistir && !redraw )
		{
			/* Desistiu! */
			score += tim_tempo * descobertas;
			proxima = 0;
			for ( i = quantas-1 ; strlen(palavras[i].palavra) == tam; i-- )
			{
				if ( palavras[i].ligado )
				{
					proxima = palavras[i].ligado;
					break;
				}
			}
			
			for ( i = 0 ; i < quantas ; i++ )
			{
				palavras[i].ligado = 1;
			}
		}
		if ( descobertas == quantas && !redraw )
		{
			/* Fim de jogo */
			score += tim_tempo * descobertas;
			proxima = 0;
			for ( i = quantas-1 ; strlen(palavras[i].palavra) == tam; i-- )
			{
				if ( palavras[i].ligado )
				{
					proxima = palavras[i].ligado;
				}
			}
			
			for ( i = 0 ; i < quantas ; i++ )
			{
				palavras[i].ligado = 1;
			}
			desistir = 1;
		}
		
		if ( tim_tempo < 0 && tempo && !redraw )
		{
			/* Cabou tempo! */
			proxima = 0;
			for ( i = quantas-1 ; strlen(palavras[i].palavra) == tam; i-- )
			{
				if ( palavras[i].ligado )
				{
					proxima = palavras[i].ligado;
				}
			}
			
			for ( i = 0 ; i < quantas ; i++ )
			{
				palavras[i].ligado = 1;
			}
			desistir = 1;
			remove_int(dec_tim_tempo);
			tim_tempo = 0;
		}
		
		if ( !redraw )
		{
			show_mouse(buffer);
		}
		blit(buffer, screen, 0, 0, 0, 0, buffer->w, buffer->h);
		
		if ( desistir && redraw )
		{
			if ( !gOver(buffer, proxima, clique) )
			{
				letroca (buffer, fundo, Letras, tempo, clique, erro);
			}
			return;
		}
	}
}

int subgrupoSort (const void *ant, const void *post)
{
	SUBGRUPOS *a, *b;
	a = (SUBGRUPOS *) ant;
	b = (SUBGRUPOS *) post;
	
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

void embaralhaFunc ( MAIOR *palavraMaior, int tam )
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

void sorteia ( MAIOR *palavraMaior, SUBGRUPOS *palavras, int *quantas )
{
	int sorteada, tam, i;
	FILE *fin;
	char linha[200];
	
	fin = fopen("dicionario.txt","r");
	tam = 0;
	fgets(linha,200,fin);
    while (!feof(fin))
	{
		tam++;
		fgets(linha,200,fin);
	}
	
	sorteada = rand() % tam;
	
	rewind(fin);
	
	for ( i = 0 ; i < sorteada ; i++ )
	{
		fgets(linha,200,fin);
	}
	
	fscanf(fin, "%s %d ",palavraMaior->palavra, quantas);
	
	for ( i = 0 ; i < *quantas ; i++ )
	{
		fscanf(fin, "%s", palavras[i].palavra);
		palavras[i].numletras = strlen(palavras[i].palavra);
	}
	
	fclose(fin);
}


void sobre (BITMAP *buffer, SAMPLE *clique)
{
	int sizeX, sizeY, janelaX, janelaY, bordaJanX, bordaJanY, botaoX, botaoY, bordaBotX, bordaBotY, pressed;
	BITMAP *buffer1;
	buffer1 = create_bitmap(MAX_X, MAX_Y);
	blit(buffer, buffer1, 0, 0, 0, 0, screen->w, screen->h);
	
	sizeX = buffer->w/2;
	sizeY = buffer->h/2;
	
	/* Janela de tamanho 280 x 200 
	   desenha um retângulo desse tamanho, 
	   depois colore por dentro, 
	   deixando uma borda de 1px
	*/
	bordaJanX = 500 / 2;
	janelaX = bordaJanX - 1;
	bordaJanY = 300 / 2;
	janelaY = bordaJanY - 1;
	
	/* Botão de 40x20 
	   Idem acima.
	*/
	bordaBotX = 40 / 2;
	botaoX = bordaBotX - 1;
	bordaBotY = 20;
	botaoY = bordaBotY - 1;
	
	mouse_b = 0;
	pressed = 0;
	
	while ( 1 )
	{
		blit(buffer1, buffer, 0, 0, 0, 0, screen->w, screen->h);
		
		
		rect(buffer, sizeX-bordaJanX, sizeY-bordaJanY, sizeX+bordaJanX, sizeY+bordaJanY, 
		makecol(255,255,255));
		rectfill(buffer, sizeX-janelaX, sizeY-janelaY, sizeX+janelaX, sizeY+janelaY, 
		makecol(0,0,0));
		
		textprintf_centre(buffer, font, sizeX, sizeY-145, makecol(255,255,255),
			"Sobre/Como jogar");
		textprintf_centre(buffer, font, sizeX, sizeY-125, makecol(255,255,255),
		"Letroca é um jogo onde o objetivo é formar todas as palavras");
		textprintf_centre(buffer, font, sizeX, sizeY-115, makecol(255,255,255),
		"possíveis com as letras disponíveis.");
		textprintf_centre(buffer, font, sizeX, sizeY-105, makecol(255,255,255),
		"É possível interagir com o programa usando teclado ou mouse.");
		textprintf_centre(buffer, font, sizeX, sizeY-95, makecol(255,255,255),
		"Comandos básicos:");
		textprintf_centre(buffer, font, sizeX, sizeY-85, makecol(255,255,255),
		"Pressionando as letras do teclado ou clicando nas mesmas,");
		textprintf_centre(buffer, font, sizeX, sizeY-75, makecol(255,255,255),
		"forma-se a palavra desejada.");
		textprintf_centre(buffer, font, sizeX, sizeY-65, makecol(255,255,255),
		"Pressionando espaço ou clicando sobre \"Embaralhar\", as");
		textprintf_centre(buffer, font, sizeX, sizeY-55, makecol(255,255,255),
		"letras serão misturadas novamente.");
		textprintf_centre(buffer, font, sizeX, sizeY-45, makecol(255,255,255),
		"Pressionando enter ou clicando sobre \"Ok\" a palavra");
		textprintf_centre(buffer, font, sizeX, sizeY-35, makecol(255,255,255),
		"é testada para ver se existe no dicionário.");
		textprintf_centre(buffer, font, sizeX, sizeY-25, makecol(255,255,255),
		"Pressionando backspace ou clicando sobre a última letra");
		textprintf_centre(buffer, font, sizeX, sizeY-15, makecol(255,255,255),
		"selecionada desfaz a última jogada.");
		textprintf_centre(buffer, font, sizeX, sizeY-5, makecol(255,255,255),
		"Ao clicar sobre \"Limpar\" todas as letras retornam para");
		textprintf_centre(buffer, font, sizeX, sizeY+5, makecol(255,255,255),
		"serem escolhidas novamente.");
		textprintf_centre(buffer, font, sizeX, sizeY+15, makecol(255,255,255),
		"Pressionando Esc ou clicando sobre \"Desistir\", abandona-se o");
		textprintf_centre(buffer, font, sizeX, sizeY+25, makecol(255,255,255),
		"estágio atual.");
		textprintf_centre(buffer, font, sizeX, sizeY+35, makecol(255,255,255),
		"Mais informações sobre o jogo na documentação.");
		textprintf_centre(buffer, font, sizeX, sizeY+55, makecol(255,255,255),
		"Por: Cinila Jesus Machado,");
		textprintf_centre(buffer, font, sizeX, sizeY+65, makecol(255,255,255),
		"Renan Moreira Lobo,");
		textprintf_centre(buffer, font, sizeX, sizeY+75, makecol(255,255,255),
		"Renato dos Santos Cerqueira");

		rect(buffer, sizeX-bordaBotX, sizeY+(janelaX/3)+bordaBotY, sizeX+bordaBotX, sizeY+(janelaX/3)+(bordaBotY*2), 
		makecol(255,255,255));
		rectfill(buffer, sizeX-botaoX, sizeY+(janelaX/3)+botaoY+2, sizeX+botaoX, sizeY+(janelaX/3)+(botaoY*2), 
		makecol(0,0,0));
			
		textprintf_centre(buffer, font, sizeX, sizeY+(janelaX/2)-bordaBotY+7, makecol(255,255,255),
			"Ok");
		
		show_mouse(buffer);
		
		if ( keypressed() )
		{
			if ( som )
			{
				play_sample(clique, 255, 128, 1000, FALSE);
			}

			pressed = readkey();
			
			if ( pressed >> 8 == KEY_ENTER || pressed >> 8 == KEY_ENTER_PAD )
			{
				break;
			}
		}
		
		if ( mouse_b == 1 )
		{
			if ( som )
			{
				play_sample(clique, 255, 128, 1000, FALSE);
			}
			if ( (mouse_x > sizeX-bordaBotX && mouse_x < sizeX+bordaBotX) && (mouse_y > sizeY+bordaBotY+(janelaX/3) && mouse_y < sizeY+(bordaBotY*2)+(janelaX/3)) )
			{
				break;
			}
		}
		
		blit(buffer, screen, 0, 0, 0, 0, buffer->w, buffer->h);
		show_mouse(NULL);
	}
	blit(buffer1, buffer, 0, 0, 0, 0, buffer->w, buffer->h);
	destroy_bitmap(buffer1);
}

void palavraNaoEncontrada (BITMAP *buffer, int tempo, SAMPLE *clique)
{
	int sizeX, sizeY, janelaX, janelaY, bordaJanX, bordaJanY, botaoX, botaoY, bordaBotX, bordaBotY, pressed;
	BITMAP *buffer1;
	buffer1 = create_bitmap(MAX_X, MAX_Y);
	blit(buffer, buffer1, 0, 0, 0, 0, screen->w, screen->h);
	mouse_b = 0;
	pressed = 0;
	
	sizeX = buffer->w/2;
	sizeY = buffer->h/2;
	
	/* Janela de tamanho 160 x 120 
	   desenha um retângulo desse tamanho, 
	   depois colore por dentro, 
	   deixando uma borda de 1px
	*/
	bordaJanX = 200 / 2;
	janelaX = bordaJanX - 1;
	bordaJanY = 60 / 2;
	janelaY = bordaJanY - 1;
	
	/* Botão de 40x20 
	   Idem acima.
	*/
	bordaBotX = 40 / 2;
	botaoX = bordaBotX - 1;
	bordaBotY = 20;
	botaoY = bordaBotY - 1;
	
	while ( 1 )
	{
		blit(buffer1, buffer, 0, 0, 0, 0, screen->w, screen->h);
		
		rect(buffer, sizeX-bordaJanX, sizeY-bordaJanY, sizeX+bordaJanX, sizeY+bordaJanY, 
		makecol(255,255,255));
		rectfill(buffer, sizeX-janelaX, sizeY-janelaY, sizeX+janelaX, sizeY+janelaY, 
		makecol(0,0,0));
		
		rect(buffer, sizeX-bordaBotX, sizeY+35, sizeX+bordaBotX, sizeY+bordaBotY+35, 
		makecol(255,255,255));
		rectfill(buffer, sizeX-botaoX, sizeY+36, sizeX+botaoX, sizeY+botaoY+35, makecol(0,0,0));
		
		textprintf_centre(buffer, font, sizeX, sizeY, makecol(255,255,255),
			"Não está no dicionario");
		
		textprintf_centre(buffer, font, sizeX, sizeY+bordaBotY+22, makecol(255,255,255),
			"Ok");
		
		show_mouse(buffer);
		
		/* Caixa de tempo */
		if ( tempo )
		{
			rectfill(buffer, buffer->w-100, 65, buffer->w-15, 100, makecol(0,0,0));
			textprintf_centre(buffer, font, buffer->w-57, 80, makecol(255,255,255),
			"%02d:%02d", tim_tempo/60, tim_tempo%60 );
		}
		
		if ( keypressed() )
		{
			if ( som )
			{
				play_sample(clique, 255, 128, 1000, FALSE);
			}
			pressed = readkey();
			if (pressed >> 8 == KEY_ENTER || pressed >> 8 == KEY_ENTER_PAD )
			{
				break;
			}
		}
		
		if ( mouse_b == 1 ) 
		{
			if ( som )
			{
				play_sample(clique, 255, 128, 1000, FALSE);
			}
			if ( (mouse_x > sizeX-bordaBotX && mouse_x < sizeX+bordaBotX) && (mouse_y > sizeY+35 && mouse_y < sizeY+bordaBotY+35) )
			{
				break;
			}
		}
		
		blit(buffer, screen, 0, 0, 0, 0, buffer->w, buffer->h);
		show_mouse(NULL);
	}
	blit(buffer1, buffer, 0, 0, 0, 0, buffer->w, buffer->h);
	destroy_bitmap(buffer1);
}

void palavraRepetida (BITMAP *buffer, int tempo, SAMPLE *clique)
{
	int sizeX, sizeY, janelaX, janelaY, bordaJanX, bordaJanY, botaoX, botaoY, bordaBotX, bordaBotY, pressed;
	BITMAP *buffer1;
	buffer1 = create_bitmap(MAX_X, MAX_Y);
	blit(buffer, buffer1, 0, 0, 0, 0, screen->w, screen->h);
	
	sizeX = buffer->w/2;
	sizeY = buffer->h/2;
	
	/* Janela de tamanho 160 x 120 
	   desenha um retângulo desse tamanho, 
	   depois colore por dentro, 
	   deixando uma borda de 1px
	*/
	bordaJanX = 200 / 2;
	janelaX = bordaJanX - 1;
	bordaJanY = 60 / 2;
	janelaY = bordaJanY - 1;
	
	/* Botão de 40x20 
	   Idem acima.
	*/
	bordaBotX = 40 / 2;
	botaoX = bordaBotX - 1;
	bordaBotY = 20;
	botaoY = bordaBotY - 1;

	mouse_b = 0;
	pressed = 0;
		
	while ( 1 )
	{
		blit(buffer1, buffer, 0, 0, 0, 0, screen->w, screen->h);
		
		rect(buffer, sizeX-bordaJanX, sizeY-bordaJanY, sizeX+bordaJanX, sizeY+bordaJanY, 
		makecol(255,255,255));
		rectfill(buffer, sizeX-janelaX, sizeY-janelaY, sizeX+janelaX, sizeY+janelaY, 
		makecol(0,0,0));
		
		rect(buffer, sizeX-bordaBotX, sizeY+35, sizeX+bordaBotX, sizeY+bordaBotY+35, 
		makecol(255,255,255));
		rectfill(buffer, sizeX-botaoX, sizeY+36, sizeX+botaoX, sizeY+botaoY+35, makecol(0,0,0));
		
		textprintf_centre(buffer, font, sizeX, sizeY, makecol(255,255,255),
			"Já foi encontrada!");
		
		textprintf_centre(buffer, font, sizeX, sizeY+bordaBotY+22, makecol(255,255,255),
			"Ok");
		
		/* Caixa de tempo */
		if ( tempo )
		{
			rectfill(buffer, buffer->w-100, 65, buffer->w-15, 100, makecol(0,0,0));
			textprintf_centre(buffer, font, buffer->w-57, 80, makecol(255,255,255),
			"%02d:%02d", tim_tempo/60, tim_tempo%60 );
		}
		
		if ( keypressed() )
		{
			if ( som )
			{
				play_sample(clique, 255, 128, 1000, FALSE);
			}
			pressed = readkey();
			if (pressed >> 8 == KEY_ENTER || pressed >> 8 == KEY_ENTER_PAD )
			{
				break;
			}
		}
		
		if ( mouse_b == 1 ) 
		{
			if ( som )
			{
				play_sample(clique, 255, 128, 1000, FALSE);
			}
			if ( (mouse_x > sizeX-bordaBotX && mouse_x < sizeX+bordaBotX) && (mouse_y > sizeY+35 && mouse_y < sizeY+bordaBotY+35) )
			{
				break;
			}
		}
		show_mouse(buffer);				
		blit(buffer, screen, 0, 0, 0, 0, buffer->w, buffer->h);
		show_mouse(NULL);
	}
	blit(buffer1, buffer, 0, 0, 0, 0, buffer->w, buffer->h);
	destroy_bitmap(buffer1);
}

int desejaSair (BITMAP *buffer, int tempo, SAMPLE *clique)
{
	int sizeX, sizeY, janelaX, janelaY, bordaJanX, bordaJanY, botaoX, botaoY, bordaBotX, bordaBotY, pressed;
	BITMAP *buffer1;
	buffer1 = create_bitmap(MAX_X, MAX_Y);
	blit(buffer, buffer1, 0, 0, 0, 0, screen->w, screen->h);

	sizeX = buffer->w/2;
	sizeY = buffer->h/2;
	
	/* Janela de tamanho 200 x 60 
	   desenha um retângulo desse tamanho, 
	   depois colore por dentro, 
	   deixando uma borda de 1px
	*/
	bordaJanX = 200 / 2;
	janelaX = bordaJanX - 1;
	bordaJanY = 60 / 2;
	janelaY = bordaJanY - 1;
	
	/* Botão de 40x20 
	   Idem acima.
	*/
	bordaBotX = 40 / 2;
	botaoX = bordaBotX - 1;
	bordaBotY = 20;
	botaoY = bordaBotY - 1;
	
	mouse_b = 0;
	pressed = 0;

	
	while ( 1 ) 
	{
		blit(buffer1, buffer, 0, 0, 0, 0, screen->w, screen->h);
		rect(buffer, sizeX-bordaJanX, sizeY-bordaJanY, sizeX+bordaJanX, sizeY+bordaJanY, 
		makecol(255,255,255));
		rectfill(buffer, sizeX-janelaX, sizeY-janelaY, sizeX+janelaX, sizeY+janelaY, 
		makecol(0,0,0));
		
		rect(buffer, sizeX-bordaBotX-40, sizeY+35, sizeX+bordaBotX-40, sizeY+bordaBotY+35, 
		makecol(255,255,255));
		rectfill(buffer, sizeX-botaoX-40, sizeY+36, sizeX+botaoX-40, sizeY+botaoY+35, makecol(0,0,0));
		
		rect(buffer, sizeX-bordaBotX+40, sizeY+35, sizeX+bordaBotX+40, sizeY+bordaBotY+35, 
		makecol(255,255,255));
		rectfill(buffer, sizeX-botaoX+40, sizeY+36, sizeX+botaoX+40, sizeY+botaoY+35, makecol(0,0,0));
		
		textprintf_centre(buffer, font, sizeX, sizeY, makecol(255,255,255),
			"Deseja mesmo desistir?");
		
		textprintf_centre(buffer, font, sizeX-40, sizeY+bordaBotY+22, makecol(255,255,255),
			"Sim");
			
		textprintf_centre(buffer, font, sizeX+40, sizeY+bordaBotY+22, makecol(255,255,255),
			"Não");
			
		/* Caixa de tempo */
		if ( tempo )
		{
			rectfill(buffer, buffer->w-100, 65, buffer->w-15, 100, makecol(0,0,0));
			textprintf_centre(buffer, font, buffer->w-57, 80, makecol(255,255,255),
			"%02d:%02d", tim_tempo/60, tim_tempo%60 );
		}
		
		show_mouse(buffer);
		
		if ( keypressed() )
		{
			if ( som )
			{
				play_sample(clique, 255, 128, 1000, FALSE);
			}
			pressed = readkey();
			if ( pressed >> 8 == KEY_ENTER || pressed >> 8 == KEY_ENTER_PAD )
			{
				return 1;
			}
			else if ( pressed >> 8 == KEY_ESC )
			{
				return 0;
			}
		}
		
		if ( mouse_b == 1 )
		{
			if ( som )
			{
				play_sample(clique, 255, 128, 1000, FALSE);
			}
			if ( (mouse_x > sizeX-bordaBotX+40 && mouse_x < sizeX+bordaBotX+40) && (mouse_y > sizeY+35 && mouse_y < sizeY+bordaBotY+35) )
			{
				return 0;
			}
			else if ( (mouse_x > sizeX-bordaBotX-40 && mouse_x < sizeX+bordaBotX-40) && (mouse_y > sizeY+35 && mouse_y < sizeY+bordaBotY+35) )
			{
				return 1;
			}
		}
		
		blit(buffer, screen, 0, 0, 0, 0, buffer->w, buffer->h);
		show_mouse(NULL);
	}
	blit(buffer1, buffer, 0, 0, 0, 0, buffer->w, buffer->h);
	destroy_bitmap(buffer1);

	return 0;
}

int gOver (BITMAP *buffer, int proxima, SAMPLE *clique)
{
	int sizeX, sizeY, janelaX, janelaY, bordaJanX, bordaJanY, botaoX, botaoY, bordaBotX, 
	bordaBotY, pressed, sai;
	BITMAP *buffer1;
	buffer1 = create_bitmap(MAX_X, MAX_Y);
	blit(buffer, buffer1, 0, 0, 0, 0, screen->w, screen->h);

	sizeX = buffer->w-100;
	sizeY = buffer->h-200;
	
	/* Janela de tamanho 100 x 200 
	   desenha um retângulo desse tamanho, 
	   depois colore por dentro, 
	   deixando uma borda de 1px
	*/
	bordaJanX = 100 / 2;
	janelaX = bordaJanX - 1;
	bordaJanY = 100 / 2;
	janelaY = bordaJanY - 1;
	
	/* Botão de 40x20 
	   Idem acima.
	*/
	bordaBotX = 40 / 2;
	botaoX = bordaBotX - 1;
	bordaBotY = 20;
	botaoY = bordaBotY - 1;
	
	mouse_b = 0;
	pressed = 0;

	while ( 1 ) 
	{
		blit(buffer1, buffer, 0, 0, 0, 0, screen->w, screen->h);
		rect(buffer, sizeX-bordaJanX, sizeY-bordaJanY, sizeX+bordaJanX, sizeY+bordaJanY, 
		makecol(255,255,255));
		rectfill(buffer, sizeX-janelaX, sizeY-janelaY, sizeX+janelaX, sizeY+janelaY, 
		makecol(0,0,0));
		
		rect(buffer, sizeX-bordaBotX, sizeY+35-50, sizeX+bordaBotX, sizeY+bordaBotY+35-50, 
		makecol(255,255,255));
		rectfill(buffer, sizeX-botaoX, sizeY+36-50, sizeX+botaoX, sizeY+botaoY+35-50, makecol(0,0,0));

		if ( proxima )
		{		
			rect(buffer, sizeX-bordaBotX-10, sizeY+(35*2)-50, sizeX+bordaBotX+10, sizeY+bordaBotY+(35*2)-50, 
			makecol(255,255,255));
			rectfill(buffer, sizeX-botaoX-10, sizeY+(36*2)-50, sizeX+botaoX+10, sizeY+botaoY+(35*2)-50, makecol(0,0,0));
		}
		
		textprintf_centre(buffer, font, sizeX, sizeY-40, makecol(255,255,255),
			"Fim de Jogo");
		
		textprintf_centre(buffer, font, sizeX, sizeY+bordaBotY+22-50, makecol(255,255,255),
			"Sair");
		
		if ( proxima )
		{
			textprintf_centre(buffer, font, sizeX, sizeY+bordaBotY+22+35-50, makecol(255,255,255),
			"Próxima");
		}
		
		show_mouse(buffer);
		
		if ( keypressed() )
		{
			if ( som )
			{
				play_sample(clique, 255, 128, 1000, FALSE);
			}
			pressed = readkey();
			if ( pressed >> 8 == KEY_ENTER || pressed >> 8 == KEY_ENTER_PAD )
			{
				sai = 1;
				break;
			}
			else if ( pressed >> 8 == KEY_ESC && proxima)
			{
				sai = 0;
				break;
			}
		}
		
		if ( mouse_b == 1 )
		{
			if ( som )
			{
				play_sample(clique, 255, 128, 1000, FALSE);
			}
			if ( (mouse_x > sizeX-bordaBotX && mouse_x < sizeX+bordaBotX) && (mouse_y > sizeY+70-50 && mouse_y < sizeY+bordaBotY+70-50) && proxima)
			{
				sai = 0;
				break;
			}
			else if ( (mouse_x > sizeX-bordaBotX && mouse_x < sizeX+bordaBotX) && (mouse_y > sizeY+35-50 && mouse_y < sizeY+bordaBotY+35-50) )
			{
				sai = 1;
				break;
			}
		}
		
		blit(buffer, screen, 0, 0, 0, 0, buffer->w, buffer->h);
		show_mouse(NULL);
	}
	blit(buffer1, buffer, 0, 0, 0, 0, buffer->w, buffer->h);
	blit(buffer1, screen, 0, 0, 0, 0, buffer->w, buffer->h);
	destroy_bitmap(buffer1);

	return sai;
}

void leRecords ( JOGADOR *recordistas )
{
	FILE *record;
	int lidos, i;
	
	record = fopen("records.dat","rb");
	if ( record )
	{
		lidos = fread(recordistas,sizeof(JOGADOR),10,record);
		if ( lidos != 10 )
		{
			perror("fread");
		}
		
		fclose(record);
	}
	else
	{
		/*perror("fopen");*/
		for ( i = 0 ; i < 10 ; i++ )
		{
			sprintf(recordistas[i].nome, "FULANO");
			recordistas[i].recorde = 100 * (10-i);
		}
	}
}
	
void guardaRecords ( JOGADOR *recordistas )
{
	FILE *record;
	int escritos;
	
	record = fopen("records.dat","wb");
	if ( !record )
	{
		perror("fopen");
		exit(EXIT_FAILURE);
	}
	
	escritos = fwrite(recordistas,sizeof(JOGADOR),10,record);
	if (escritos != 10)
	{
		perror("fwrite");
	}
	fclose(record);
}	
	
int recordsSort (const void *ant, const void *post)
{
	JOGADOR *a, *b;
	a = (JOGADOR *) ant;
	b = (JOGADOR *) post;
	
	if ( a->recorde < b->recorde )
	{
		return 1;
	}
	else if ( a->recorde > b->recorde )
	{
		return -1;
	}
	else
	{
		/* MUDAR NO DIA QUE FOR APRESENTAR */
		return strcasecmp(a->nome,b->nome);
	}
}

void novoRecord ( BITMAP *buffer, JOGADOR *recordistas, SAMPLE *clique )
{
	int sizeX, sizeY, janelaX, janelaY, bordaJanX, bordaJanY, botaoX, botaoY, bordaBotX, bordaBotY, pressed,
	bordaEspX, bordaEspY, espacoX, espacoY, caracs;
	BITMAP *buffer1;
	char nome[40];
	
	nome[0] = '\0';
	
	caracs = 0;
	
	buffer1 = create_bitmap(MAX_X, MAX_Y);
	blit(screen, buffer1, 0, 0, 0, 0, screen->w, screen->h);

	sizeX = buffer->w/2;
	sizeY = buffer->h/2;
	
	/* Janela de tamanho 100 x 200 
	   desenha um retângulo desse tamanho, 
	   depois colore por dentro, 
	   deixando uma borda de 1px
	*/
	bordaJanX = 330 / 2;
	janelaX = bordaJanX - 1;
	bordaJanY = 100 / 2;
	janelaY = bordaJanY - 1;
	
	/* Espaço pra escrever 190x20
	*/
	bordaEspX = 324 / 2;
	espacoX = bordaEspX - 1;
	bordaEspY = 20 / 2;
	espacoY = bordaEspY - 1;
	
	/* Botão de 40x20 
	   Idem acima.
	*/
	bordaBotX = 40 / 2;
	botaoX = bordaBotX - 1;
	bordaBotY = 20;
	botaoY = bordaBotY - 1;
	
	mouse_b = 0;
	pressed = 0;

	
	while ( 1 ) 
	{
		blit(buffer1, buffer, 0, 0, 0, 0, screen->w, screen->h);
		rect(buffer, sizeX-bordaJanX, sizeY-bordaJanY, sizeX+bordaJanX, sizeY+bordaJanY, 
		makecol(255,255,255));
		rectfill(buffer, sizeX-janelaX, sizeY-janelaY, sizeX+janelaX, sizeY+janelaY, 
		makecol(0,0,0));
		
		rect(buffer, sizeX-bordaEspX, sizeY+35-50, sizeX+bordaEspX, sizeY+bordaEspY+35-50, 
		makecol(255,255,255));
		rectfill(buffer, sizeX-espacoX, sizeY+36-50, sizeX+espacoX, sizeY+espacoY+35-50, makecol(0,0,0));

		rect(buffer, sizeX-bordaBotX, sizeY+bordaBotY, sizeX+bordaBotX, sizeY+(bordaBotY*2), 
		makecol(255,255,255));
		rectfill(buffer, sizeX-botaoX, sizeY+botaoY+2, sizeX+botaoX, sizeY+(botaoY*2), 
		makecol(0,0,0));

		textprintf_centre(buffer, font, sizeX, sizeY-40, makecol(255,255,255),
			"Você fez um novo recorde!");
			
		textprintf_centre(buffer, font, sizeX, sizeY-13, makecol(255,255,255),
		    "%s",nome);
		
		textprintf_centre(buffer, font, sizeX, sizeY+bordaBotY+7, makecol(255,255,255),
			"Ok");
		
		
		show_mouse(buffer);
		
		if ( keypressed() )
		{
			if ( som )
			{
				play_sample(clique, 255, 128, 1000, FALSE);
			}
			pressed = readkey();
			if ( pressed >> 8 == KEY_ENTER || pressed >> 8 == KEY_ENTER_PAD || pressed >> 8 == KEY_ESC)
			{
				break;
			}
			else if ( (pressed >> 8) >= KEY_A && (pressed >> 8) <= KEY_Z && caracs < 39)
			{
				nome[caracs] = (pressed >> 8) - 1 + 'A';
				if ( caracs < 39 )
				{
					nome[caracs+1] = '\0';
				}
				caracs++;
			}
			else if ( (pressed >> 8) == KEY_SPACE )
			{
				nome[caracs] = ' ';
				caracs++;
			}
			else if ( (pressed >> 8) == KEY_BACKSPACE )
			{
				if ( caracs > 0 )
				{
					nome[caracs-1] = '\0';
					caracs--;
				}
			}
		}
		
		if ( mouse_b == 1 )
		{
			if ( som )
			{
				play_sample(clique, 255, 128, 1000, FALSE);
			}
			if ( (mouse_x > sizeX-bordaBotX && mouse_x < sizeX+bordaBotX) && (mouse_y > sizeY+bordaBotY && mouse_y < sizeY+(bordaBotY*2)) )
			{
				break;
			}
		}
		
		blit(buffer, screen, 0, 0, 0, 0, buffer->w, buffer->h);
		show_mouse(NULL);
	}
	if ( caracs == 0 )
	{
		strcpy(nome,"SEM NOME\0");
	}
	
	strcpy(recordistas[10].nome,nome);
	recordistas[10].recorde = score;
	
	qsort(recordistas, 11, sizeof(JOGADOR),recordsSort);
	
	blit(buffer1, buffer, 0, 0, 0, 0, buffer->w, buffer->h);
}

void exibeRecord ( BITMAP *buffer, JOGADOR *recordistas, SAMPLE *clique )
{
	int sizeX, sizeY, janelaX, janelaY, bordaJanX, bordaJanY, botaoX, botaoY, bordaBotX, 
	bordaBotY, pressed, i;
	BITMAP *buffer1;
	
	buffer1 = create_bitmap(MAX_X, MAX_Y);
	blit(screen, buffer1, 0, 0, 0, 0, screen->w, screen->h);

	sizeX = buffer->w/2;
	sizeY = buffer->h/2;
	
	/* Janela de tamanho 500 x 300 
	   desenha um retângulo desse tamanho, 
	   depois colore por dentro, 
	   deixando uma borda de 1px
	*/
	bordaJanX = 500 / 2;
	janelaX = bordaJanX - 1;
	bordaJanY = 300 / 2;
	janelaY = bordaJanY - 1;
	
	/* Botão de 40x20 
	   Idem acima.
	*/
	bordaBotX = 40 / 2;
	botaoX = bordaBotX - 1;
	bordaBotY = 20;
	botaoY = bordaBotY - 1;
	
	mouse_b = 0;
	pressed = 0;

	
	while ( 1 ) 
	{
		blit(buffer1, buffer, 0, 0, 0, 0, screen->w, screen->h);
		rect(buffer, sizeX-bordaJanX, sizeY-bordaJanY, sizeX+bordaJanX, sizeY+bordaJanY, 
		makecol(255,255,255));
		rectfill(buffer, sizeX-janelaX, sizeY-janelaY, sizeX+janelaX, sizeY+janelaY, 
		makecol(0,0,0));
		
		rect(buffer, sizeX-bordaBotX, sizeY+(janelaX/3)+bordaBotY, sizeX+bordaBotX, sizeY+(janelaX/3)+(bordaBotY*2), 
		makecol(255,255,255));
		rectfill(buffer, sizeX-botaoX, sizeY+(janelaX/3)+botaoY+2, sizeX+botaoX, sizeY+(janelaX/3)+(botaoY*2), 
		makecol(0,0,0));

		textprintf_centre(buffer, font, sizeX, sizeY-(janelaX/2)-15, makecol(255,255,255),
			"Tabela de Recordes - Top10");
			
		for ( i = 0 ; i < 10 ; i++ )
		{
			textprintf_centre(buffer, font, sizeX, sizeY-(janelaX/2)+(20*(i+1)), 
			makecol(255,255,255), "%d) %s - %d",i+1,recordistas[i].nome, recordistas[i].recorde);
		}
		
		textprintf_centre(buffer, font, sizeX, sizeY+(janelaX/2)-bordaBotY+7, makecol(255,255,255),
			"Ok");
		
		
		show_mouse(buffer);
		
		if ( keypressed() )
		{
			if ( som )
			{
				play_sample(clique, 255, 128, 1000, FALSE);
			}
			pressed = readkey();
			if ( pressed >> 8 == KEY_ENTER || pressed >> 8 == KEY_ENTER_PAD || pressed >> 8 == KEY_ESC)
			{
				break;
			}
		}
		
		if ( mouse_b == 1 )
		{
			if ( som )
			{
				play_sample(clique, 255, 128, 1000, FALSE);
			}
			if ( (mouse_x > sizeX-bordaBotX && mouse_x < sizeX+bordaBotX) && (mouse_y > sizeY+bordaBotY+(janelaX/3) && mouse_y < sizeY+(bordaBotY*2)+(janelaX/3)) )
			{
				break;
			}
		}
		
		blit(buffer, screen, 0, 0, 0, 0, buffer->w, buffer->h);
		show_mouse(NULL);
	}
	
	blit(buffer1, buffer, 0, 0, 0, 0, buffer->w, buffer->h);
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
