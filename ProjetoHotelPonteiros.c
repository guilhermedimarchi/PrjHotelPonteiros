#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct		hospede{
	int			reg_hosp; 		// gerado automaticamente
	char			nome[80];
	int			num_pessoas;	//nº de pessoas no quarto – Máx. 3
	char			sigla;			// [A]lugado ou [R]eservado
	int			num_quarto;	// número do quarto – buscar no arq. quarto
}hospede;


typedef struct info_hospede{
	char sigla;			// O - Ocupado ou R - Reservado
	int	reg_hosp;		// reg. do hóspede - buscar no arq. hóspede
	int	dia_in;			// dia - máx=31
	int	mes_in;			// mes: fev=28 dias
	int	dia_out;		// mes: abril=jun=set=nov=30 dias
	int	mes_out;		// os demais = 31 dias
}info_hospede;


typedef union estado{
	char sigla;		// L - Livre
	info_hospede dados[2];	//um para ocupacao e o outro para reserva
}estado;

typedef struct quarto{
	int	num_quarto;		// gerado automaticamente
	char categoria;		// [S]olteiro ou [F]amiliar
	float diaria;
	int gravacoes;
	estado status;

} quarto;


void inicializaquartos();
int gravaQuarto(info_hospede info_h, int pos);
void alocaQuarto(quarto **aux, int tam);
void alocaint(int **aux, int tam);
void mostraTotal();
void mostraTotalHospede(int qtde);
 info_hospede mostraParcial(hospede h,int **possiveisQuartos,int *aux_cont);
int convertedata(int dia, int mes);
void checkin(hospede h, int qtde,int indice,int **possiveisQuartos,info_hospede info_h,int *aux_cont);
void grava(hospede h, int pos,info_hospede info_h,int status);
int verifica();
void atualizaQuarto(int pos, char sigla);
void checkout();
int calculaPreco(int pos);
void liberaQuarto(int pos);
void tiraHospede(int pos, char sigla);
int desativado(int qtde);
int validaCheckout(int n);

main() {
    int op,qtde,*possiveisQuartos=NULL, *aux_cont=NULL,indice=-1;
	inicializaquartos(); //CRIA OS 20 QUARTOS
    hospede h;
    info_hospede info_h;

    alocaint(&aux_cont,1); //CRIA OS 20 QUARTOS
	do
    {
        system("PAUSE");
        system("CLS");
        qtde = verifica();//Verifica quantidade de hospedes
        printf("[1]-Check-in\n[2]-Check-out\n[3]-Mostra Quartos total\n[4]-Mostra hospede total\n[5]-Sair\nOp: ");
        scanf("%i",&op);
        fflush(stdin);

        switch(op)
        {
            case 1:
                 indice = desativado(qtde); //Encontra espaço para colocar hospede

                 if(indice == -1)
                     checkin(h,qtde,qtde,&possiveisQuartos,info_h,aux_cont);
                 else
                     checkin(h,qtde,indice,&possiveisQuartos,info_h,aux_cont);

                break;
            case 2:
                checkout();
                break;
            case 3:
                mostraTotal();
                break;
            case 4:
                mostraTotalHospede(qtde);
                break;
        }

    }while(op!=5);
	system("PAUSE");
}

int validaCheckout(int n) //Verifica se o checkout é valido
{

    FILE *fq=NULL;
    int valido=0;
	quarto q;
    if((fq = fopen("quarto.bin","rb+"))==NULL)
	{
		printf("ERRO");
	}
	else
	{
            fseek(fq,(n-1)*sizeof(quarto),0);
            fread(&q, sizeof(quarto),1,fq);

           	if(q.status.sigla == 'L')
           	    valido = 0;
            else
                valido = 1;

        fclose(fq);
	}

return valido;
}



int desativado(int qtde)//Procura qual indice tem um espaço para alocar um hospede novo
{

 FILE *fp=NULL; // Hospede.bin
    hospede h;
    int i;

    if((fp=fopen("hospede.bin","rb+"))==NULL)
    {
        printf("ERRO");
    }
    else
    {
        for(i=0;i<qtde;i++)
        {
                        fseek(fp,i*sizeof(hospede),0);
                        fread(&h,sizeof(hospede),1,fp);
                        if(h.sigla == '-')
                                   return i;
        }
        fclose(fp);
    }
    return -1;
}

void checkout()
{
    int n,valido;
    int preco;


    do
    {
     valido=0;
     printf("Numero do quarto para checkout: ");
     scanf("%i",&n);
     fflush(stdin);
     valido = validaCheckout(n);

     }while(valido==0);

    preco = calculaPreco(n-1);
    printf("Preco: %i\n",preco);

    liberaQuarto(n-1);

}

void tiraHospede(int pos, char sigla)//Muda a sigla do hospede para - ou O ou R
{
    FILE *fp=NULL; // Hospede.bin
    hospede h;

    if((fp=fopen("hospede.bin","rb+"))==NULL)
    {
        printf("ERRO");
    }
    else
    {
        fseek(fp,pos*sizeof(hospede),0);
        fread(&h,sizeof(hospede),1,fp);

        h.sigla = sigla;
//        printf("\n%c\n", h.sigla);

        fseek(fp,pos*sizeof(hospede),0);
        fwrite(&h,sizeof(hospede),1,fp);
        fclose(fp);
    }


}

void liberaQuarto(int pos)//Tira o Ocupacao e coloca a Reserva no lugar
{
    FILE *fq=NULL;
    int reg_h;
	quarto q;
    if((fq = fopen("quarto.bin","rb+"))==NULL)
	{
		printf("ERRO");
	}
	else
	{
        fseek(fq,pos*sizeof(quarto),0);
        fread(&q, sizeof(quarto),1,fq);

        reg_h = q.status.dados[0].reg_hosp;



        tiraHospede(reg_h-1,'-'); //Muda o h.sigla


        q.gravacoes--;

        if(q.gravacoes==0)
            q.status.sigla = 'L';
        else
        {
            reg_h = q.status.dados[1].reg_hosp;
            q.status.dados[0] = q.status.dados[1];
            tiraHospede(reg_h-1,'O');
        }

        fseek(fq,pos*sizeof(quarto),0);
        fwrite(&q,sizeof(quarto),1,fq);

        fclose(fq);
	}

}


int calculaPreco(int pos)
{

    FILE *fq=NULL;
    int preco;
	quarto q;
    if((fq = fopen("quarto.bin","rb+"))==NULL)
	{
		printf("ERRO");
	}
	else
	{
        fseek(fq,pos*sizeof(quarto),0);
        fread(&q, sizeof(quarto),1,fq);

        if(      ((q.status.dados)->mes_out) >= ((q.status.dados)->mes_in)          )
                  preco = q.diaria * (   convertedata(((q.status.dados))->dia_out,((q.status.dados))->mes_out) - convertedata(((q.status.dados))->dia_in,((q.status.dados))->mes_in)       + 1 );
        else//CASO hospede entrar em dezembro e sair em janeiro
        {
            preco = q.diaria * (   convertedata(31,12) - convertedata(   (q.status.dados)->dia_in,  (q.status.dados)->mes_in     ) );
            preco += q.diaria * (   convertedata(    (q.status.dados)->dia_out,    (q.status.dados)->mes_out     )    +1  );
        }
        fclose(fq);
	}

    return preco;

}



void checkin(hospede h, int qtde,int indice,int **possiveisQuartos,info_hospede info_h,int *aux_cont)
{
    int status,valido,i;
    h.reg_hosp = indice+1;

    printf("Digite seu nome: ");
    gets(h.nome);
    fflush(stdin);
    do{
    printf("Quantidade de acompanhantes: ");
    scanf("%i",&h.num_pessoas);
    fflush(stdin);
    }while(h.num_pessoas<0 ||h.num_pessoas>3);
    h.sigla = '-1';
    info_h = mostraParcial(h,possiveisQuartos,aux_cont); //cria vetor com possiveis quartos
    info_h.reg_hosp = h.reg_hosp;



    do
    {
        valido=0;
        printf("\nQual quarto deseja?: ");
        scanf("%i",&h.num_quarto);
        fflush(stdin);

        for(i=0;i<(*(aux_cont));i++)
        {
            if(h.num_quarto == *((*possiveisQuartos)+i))
                valido=1;
        }


    }while(valido!=1); //valida se a entrada é possivel

    atualizaQuarto((h.num_quarto-1),'O'); //Coloca o quarto como Ocupado
    status = gravaQuarto(info_h,h.num_quarto-1);//grava alteracao no quarto
    grava(h,indice,info_h,status);//grava alteração no hospede
}



void atualizaQuarto(int pos, char sigla)//Coloca o quarto como Ocupado
{
    FILE *fq=NULL;
	quarto q;
    if((fq = fopen("quarto.bin","rb+"))==NULL)
	{
		printf("ERRO");
	}
	else
	{
        fseek(fq,pos*sizeof(quarto),0);
        fread(&q, sizeof(quarto),1,fq);

        (q.status.dados)->sigla = sigla;


        fseek(fq,pos*sizeof(quarto),0);
        fwrite(&q,sizeof(quarto),1,fq);
        fclose(fq);
	}
}



void mostraTotalHospede(int qtde)
{
	FILE *fq=NULL;
	FILE *fi=NULL;
	hospede q;
	info_hospede h;
	int i;

	if((fq = fopen("hospede.bin","rb"))==NULL)
	{
		printf("ERRO");

	}
	else
	{
	    printf("Reg.\t|Nome\t\t|Ocupantes\t|Sigla\t|Quarto\t|Entrada|Saida ");
		for(i=0;i<qtde;i++)
		{
			fseek(fq,i*sizeof(hospede),0);
			fread(&q, sizeof(hospede),1,fq);

			printf("\n%i\t|%s\t\t|\%i\t\t|%c\t|%i\t|",q.reg_hosp,q.nome,q.num_pessoas,q.sigla,q.num_quarto);


                        if((fi = fopen("info_hospede.bin","rb"))==NULL)
                            printf("ERRO");
                        else
                        {
                            fseek(fi,i*sizeof(info_hospede),0);
                            fread(&h, sizeof(info_hospede),1,fi);


                            printf("%i/%i\t|%i/%i", h.dia_in,h.mes_in,h.dia_out,h.mes_out);
                            fclose(fi);
                        }
		}
		fclose(fq);
	}
}


info_hospede mostraParcial(hospede h, int **possiveisQuartos,int *aux_cont)
{


    FILE *fq=NULL;
    quarto q;
    info_hospede info_h;
    int aux_dia_in,aux_mes_in, aux_dia_out, aux_mes_out,i,cont=0;
    char aux_cat;
    int valido=1;


    if((fq = fopen("quarto.bin","rb"))==NULL)
        printf("ERRO");
    else
    {
        do
        {
            printf("Data de Entrada: (dd/mm)");
            scanf("%i/%i",&aux_dia_in,&aux_mes_in);
            fflush(stdin);

            printf("Data de Saida: (dd/mm)");
            scanf("%i/%i",&aux_dia_out,&aux_mes_out);
            fflush(stdin);

            info_h.dia_in = aux_dia_in;
            info_h.dia_out = aux_dia_out;
            info_h.mes_in = aux_mes_in;
            info_h.mes_out = aux_mes_out;
            info_h.sigla = 'O';

        }while(
               ((aux_mes_out<aux_mes_in && (aux_mes_in!=12 && aux_mes_out!=1)    ) ||
                (aux_mes_out==aux_mes_in && aux_dia_in>aux_dia_out)) ||
               ((aux_mes_in==4 ||aux_mes_in==6 ||aux_mes_in==9 ||aux_mes_in==11)&&(aux_dia_in>30)) ||
               ((aux_mes_out==4 ||aux_mes_out==6 ||aux_mes_out==9 ||aux_mes_out==11)&&(aux_dia_out>30)) ||
               ((aux_mes_in==2) && (aux_dia_in>28)) ||
               ((aux_mes_out==2) && (aux_dia_out>28)) ||
                ((aux_mes_in<1 || aux_mes_in>12)) ||
               ((aux_mes_out<1 || aux_mes_out>12)) ||
               ((aux_dia_in<1 || aux_dia_in>31)) ||
               ((aux_dia_out<1 || aux_dia_out>31))); //valida se o mes e dia realmente existe

        if(h.num_pessoas==0)
            aux_cat = 'S';
        else
            aux_cat = 'F';


        printf("\nQuartos Livres nessa data e categoria: ");
        for(i=0;i<20;i++)
        {
            valido = 0;
            fseek(fq,i*sizeof(quarto),0);
            fread(&q,sizeof(quarto),1,fq);

            if(q.categoria == aux_cat)
            {

                if(q.gravacoes==0) //Se gravaçoes ==0 quarto L, gravacoes ==1 quarto ocupado e gravaçoes == 2 quarto reservado
                {
                    valido = 1;
                }
                else if(q.gravacoes==1)
                {
                    if(

                          ((((convertedata(aux_dia_in,aux_mes_in) < convertedata(((q.status.dados))->dia_in,((q.status.dados))->mes_in))
                          &&
                          (convertedata(aux_dia_out,aux_mes_out) < convertedata(((q.status.dados))->dia_out,((q.status.dados))->mes_out)))

                          ||

                          ((convertedata(aux_dia_in,aux_mes_in) > convertedata(((q.status.dados))->dia_in,((q.status.dados))->mes_in))
                          &&
                          (convertedata(aux_dia_out,aux_mes_out) > convertedata(((q.status.dados))->dia_out,((q.status.dados))->mes_out)))
                          )))
                          {
                            valido=1;
                          }
                        else
                        {
                            valido=0;
                        }
                }
                else if(q.gravacoes==2)
                    valido = 0;
            }
            if(valido == 1)
            {
                alocaint(possiveisQuartos,cont+1);
                *(*(possiveisQuartos)+cont) = i+1;
                printf("\nQuarto: %i",*(*(possiveisQuartos)+cont));//VETOR DE POSSIVEIS QUARTOS
                cont++;
            }
        }
       fclose(fq);
    }
    *aux_cont = cont;
return info_h;
}


int gravaQuarto(info_hospede info_h, int pos)
{
int reg_h;
    FILE *fq=NULL;
	quarto q;
    if((fq = fopen("quarto.bin","rb+"))==NULL)
	{
		printf("ERRO");
	}
	else
	{
        fseek(fq,pos*sizeof(quarto),0);
        fread(&q, sizeof(quarto),1,fq);

        if(q.gravacoes==0)
            q.status.dados[0] = info_h;
        else if(q.gravacoes==1)
             q.status.dados[1] = info_h;

        reg_h = q.status.dados[0].reg_hosp;
  //      printf("\nREG:%i\n",reg_h);
        reg_h = q.status.dados[1].reg_hosp;
//        printf("\nREG2:%i\n",reg_h);

        q.gravacoes++;

        fseek(fq,pos*sizeof(quarto),0);
        fwrite(&q,sizeof(quarto),1,fq);
        fclose(fq);
	}
	return q.gravacoes;
}


void grava(hospede h, int pos, info_hospede info_h, int status)
{
    FILE *fp=NULL; // Hospede.bin
    FILE *fi=NULL; // Info_Hospede.bin

    if((fp=fopen("hospede.bin","rb+"))==NULL)
    {
        printf("ERRO");
    }
    else
    {
        if(status==1)
            h.sigla = 'O';
        else if(status==2)
            h.sigla = 'R';

        fseek(fp,pos*sizeof(hospede),0);
        fwrite(&h,sizeof(hospede),1,fp);
        fclose(fp);
    }




    if((fi=fopen("info_hospede.bin","rb+"))==NULL)
    {
        printf("ERRO");
    }
    else
    {
        fseek(fi,pos*sizeof(info_hospede),0);
        fwrite(&info_h,sizeof(info_hospede),1,fi);
        fclose(fi);
    }
}

void mostraTotal()
{
	FILE *fq=NULL;
	quarto q;
	int i;

	if((fq = fopen("quarto.bin","rb"))==NULL)
	{
		printf("ERRO");

	}
	else
	{
		for(i=0;i<20;i++)
		{
			fseek(fq,i*sizeof(quarto),0);
			fread(&q, sizeof(quarto),1,fq);
            if(q.status.sigla == 'L')
                printf("Numero quarto: %i\nCategoria: %c\nDiaria: %.2f\nStatus: %c\n\n\n",q.num_quarto,q.categoria,q.diaria,q.status.sigla);
            else
                printf("Numero quarto: %i\nCategoria: %c\nDiaria: %.2f\nStatus: %c\n\n\n",q.num_quarto,q.categoria,q.diaria,(q.status.dados)->sigla);
		}
		fclose((fq));
	}
}


void inicializaquartos()
{
	int i;
	FILE *fq=NULL;
	quarto q;
	//alocaQuarto(q,20);

	if((fq = fopen("quarto.bin","rb"))==NULL)
	{
		fq = fopen("quarto.bin","ab");
		for(i=0;i<20;i++)
		{
			fseek(fq,i*sizeof(quarto),0);
			q.num_quarto=i+1;
			q.status.sigla='L';
            q.gravacoes =0;
			if(i<10)
			{
				q.categoria='S';
				q.diaria = 100;
			}
			else
			{
				q.categoria='F';
				q.diaria = 150;
			}
			fwrite(&q,sizeof(quarto),1,fq);
		}
	}
	fclose(fq);
}

int convertedata(int dia, int mes){

//TRANSFORMA DATA EM UM NUMERO INTEIRO CORRESPONDENTE entre 1 - 365
int i, dias=dia;
if (mes>1)
{
    dias+=31;
    for(i=2;i<mes;i++)

    if(i==3||i==5||i==7||i==8||i==10||i==12)
         dias+=31;
    else if(i==4||i==6||i==9||i==11)
        dias+=30;
    else
        dias+=28;
}
    //printf("Dias: %i",dias);
    //system("Pause");
    return dias;

}
void alocaint(int **aux, int tam)
{
    if((*aux=(int*)realloc(*aux,tam*sizeof(int)))==NULL)
        exit(1);
}

void alocaQuarto(quarto **aux, int tam)
{
    if((*aux=(quarto*)realloc(*aux,tam*sizeof(quarto)))==NULL)
        exit(1);
}

int verifica()
{
    FILE *fp = NULL;
    FILE *fi=NULL;
    int qtde=0;

    if((fp=fopen("hospede.bin","rb"))==NULL)
    {
        fp=fopen("hospede.bin","ab");
        fi=fopen("info_hospede.bin","ab");
    }
    else
    {
            fseek(fp,0,2);
            qtde = ftell(fp)/sizeof(hospede);
    }
    fclose(fp);
    fclose(fi);
    return qtde;
}