#include <stdio.h>
#include <allegro.h>

/* Constantes */

#define MAX_X 640
#define MAX_Y 480
#define CORES 16
#define NUM_LET 7

/* Globais */
int som = 1;
volatile int ctrl_vel;

/* Escopos */

int inicializa ( void );
void finaliza ( void );

void jogo ( void );

void intro ( BITMAP *buffer );
void menu ( BITMAP *buffer );
void sobre ( BITMAP *buffer );

void inc_ctrl_vel (void);

/* Main */

int main (void)
{
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

/* Fun��es de In�cio e Fim da biblioteca */

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

	set_color_depth(CORES);
	if (set_gfx_mode(GFX_AUTODETECT, MAX_X, MAX_Y, 0, 0) < 0)
	{
		return FALSE;
	}

	return TRUE;
}


void finaliza (void)
{
	allegro_exit();
}

/* In�cio das fun��es do jogo */

void jogo (void)
{
	BITMAP *buffer;
	
	buffer = create_bitmap(MAX_X, MAX_Y);
	if ( !buffer )
	{
		return;
	}
	
	clear_to_color(screen,makecol(0,0,0));
	
	ctrl_vel = 0;
	LOCK_VARIABLE(ctrl_vel);
	LOCK_FUNCTION(inc_ctrl_vel);
	install_int(inc_ctrl_vel, 1000 / 5);
	
	intro(buffer);
	
	clear_to_color(screen,makecol(0,0,0));
	
	menu(buffer);
}

void intro (BITMAP *buffer)
{
	int fade,i, j;
	BITMAP *letras[NUM_LET];
	char nome[20];
	
	/* Carrega os sprites das palavras na mem�ria */
	for ( i = 0 ; i < NUM_LET ; i++ )
	{
		sprintf(nome,"letroca%d.bmp",i+1);
		letras[i] = load_bitmap(nome,NULL);
		if (!letras[i])
		{
			printf("Falhou\n");
			return;
		}
	}
	
	j = 0;
	
	/* Fade � a vari�vel que controla se a cor est� ficando mais clara ou mais escura
	i � a vari�vel que cont�m a cor atual */
	fade = -8;
	i = 128;
	
	/* Faz o fundo do texto ficar transparente e limpa o buffer do teclado */
	text_mode(-1);
	clear_keybuf();
	
	ctrl_vel = 0;
	
	while ( 1 )
	{
		if ( ctrl_vel > 0 )
		{
			clear_to_color(buffer,makecol(0,128,0));
			textout_centre(buffer, font, "Pressione ENTER para continuar", buffer->w / 2,
			buffer->h - 80, makecol(0,i,0));
			i += fade;
			if ( i < 0 || i > 128)
			{
				fade *= -1;
				/* Se i for menor do que 0, ent�o i = 0, se n�o, i = 128 */
				i = i<0 ? 0 : 128;
			}
			
			draw_sprite(buffer,letras[j],(buffer->w/2)-(letras[j]->w/2),30);
			j = ( j + 1 ) % NUM_LET;
			
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

void menu (BITMAP *buffer)
{
	BITMAP *sairb, *menub, *sobreb, *jogob, *jogotempob;

		/* carrega os bot�es na mem�ria */
	menub = load_bitmap("menu.bmp",NULL);
	sairb  = load_bitmap("sair.bmp",NULL);
	sobreb = load_bitmap("sobre.bmp",NULL);
	jogob = load_bitmap("jogo.bmp",NULL);
	jogotempob = load_bitmap("jogotempo.bmp",NULL);
	
	while ( 1 )
	{
		show_mouse(NULL);
		clear_to_color(buffer,makecol(0,128,0));
		
		/* desenha os bot�es */
		draw_sprite(buffer,menub,222,60);
		draw_sprite(buffer,jogob,70,161);
		draw_sprite(buffer,jogotempob,350,237);
		draw_sprite(buffer,sobreb,70,314);
		draw_sprite(buffer,sairb,350,392);
		
		/* se o bot�o direito for apertado */
		if ( ( mouse_b % 2 ) == 1 )
		{
			/* se est� na coluna da esquerda */
			if ( mouse_x >= 70 && mouse_x <= 270 ) 
			{
				/* se est� sobre o "novo jogo" */
				if ( mouse_y >= 161 && mouse_y <= 236 )
				{
				}
				/* se est� sobre o "sobre" */
				else if ( mouse_y >= 314 && mouse_y <= 389 )
				{
					sobre(buffer);
				}
			}
			
			/* se est� na coluna da direita */
			else if ( mouse_x >= 350 && mouse_x <= 450 ) 
			{
				/* se est� sobre o "jogo com tempo" */
				if ( mouse_y >= 237 && mouse_y <= 312 )
				{
				}
				/* se est� sobre o "sair" */
				else if ( mouse_y >= 392 && mouse_y <= 467 )
				{
					break;
				}
			}
		}
		
		show_mouse(buffer);
		
		blit(buffer, screen, 0, 0, 0, 0, buffer->w, buffer->h);
	}
}

void sobre (BITMAP *buffer)
{
	/* Janela de tamanho 280x200 
	   desenha um ret�ngulo desse tamanho, 
	   depois colore por dentro, 
	   deixando uma borda de 1px
	*/
	rect(buffer,(buffer->w/2)-140,(buffer->h/2)-100,(buffer->w/2)+140,(buffer->h/2)+100,makecol(255,255,255));
	rectfill(buffer,(buffer->w/2)-139,(buffer->h/2)-99,(buffer->w/2)+139,(buffer->h/2)+99,makecol(0,0,0));
	
	/* Bot�o de 40x20 
	   Idem acima.
	*/
	rect(buffer,(buffer->w/2)-20,(buffer->h/2)+60,(buffer->w/2)+20,(buffer->h/2)+80,makecol(255,255,255));
	rectfill(buffer,(buffer->w/2)-19,(buffer->h/2)+61,(buffer->w/2)+19,(buffer->h/2)+79,makecol(0,0,0));
	mouse_b = 0;
	while ( mouse_b != 1 )
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
