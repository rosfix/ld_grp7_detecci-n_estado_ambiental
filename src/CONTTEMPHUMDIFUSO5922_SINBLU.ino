//***********************************************************************                     
//***********************************************************************
#define FIS_TYPE float
#define FIS_RESOLUSION 101
#define FIS_MIN -3.4028235E+38
#define FIS_MAX 3.4028235E+38
typedef FIS_TYPE(*_FIS_MF)(FIS_TYPE, FIS_TYPE*);
typedef FIS_TYPE(*_FIS_ARR_OP)(FIS_TYPE, FIS_TYPE);
typedef FIS_TYPE(*_FIS_ARR)(FIS_TYPE*, int, _FIS_ARR_OP);
//***********************************************************************  
//***********************************************************************

// Número de entradas al sistema de inferencia difusa
const int fis_gcI = 2;
// Número de salidas al sistema de inferencia difusa
const int fis_gcO = 1;
// Número de reglas para el sistema de inferencia difusa
const int fis_gcR = 16;

FIS_TYPE g_fisInput[fis_gcI];
FIS_TYPE g_fisOutput[fis_gcO];

//DECLARO EL SENSOR 
float temperatura;
float humedad;
float salida;
#include <dht.h>
dht DHT;
#define DHT11_PIN 8

//DECLARO PINES DE LED RGB
int rojo=3;
int verde=4;
int azul=5;

//Interfaz Bluetooh
#include<SoftwareSerial.h>
SoftwareSerial BTSerial(2,3);

//Interfaz para Pantalla LCD
#include<Wire.h>
#include<LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);
String menSalida;

// Setup 
void setup()
{
    Serial.begin(9600);
    //INICIO EL LCD
    lcd.init();
    lcd.backlight();
    lcd.clear();   
}
void loop()
{
       //LECTURA DEL SENSOR DHT11
       DHT.read11(DHT11_PIN);
        
       //TEMPERATURA//
       Serial.print("Temperatura = ");
       Serial.print(DHT.temperature);
       temperatura=DHT.temperature;
       Serial.println(" C");
        
      //HUMEDAD RELATIVA//
      Serial.print("Humedad = ");
      Serial.print(DHT.humidity);
      humedad=DHT.humidity;
      Serial.println(" %");
      delay(1000);

      if(temperatura>1 && temperatura<42 && humedad>1 && humedad<100){  //Dentro de los parametros del controlador difuso
           //CARGAR LAS VARIABLES DE ENTRADA AL CONTROLADOR DIFUSO
          // Read Input: temperatura
          g_fisInput[0] = temperatura;
          // Read Input: Humedad
          g_fisInput[1] = humedad;
          g_fisOutput[0] = 0;
          fis_evaluate();
      
          // SETEAR VALOR DE SALIDA EN SALIDA
          
          Serial.print("Salida: ");
          Serial.print(g_fisOutput[0]);
          salida=g_fisOutput[0];
          if(salida>=0 && salida<=0.4){
             if(salida>=0.3 && salida<=0.4){
              Serial.print(" - CLIMA ENTRE MALO Y NORMAL ");
              menSalida=" CLIMA:MyN";
              analogWrite(rojo,255);
              analogWrite(verde,255);
              analogWrite(azul,0);
             }else{
              Serial.print(" - CLIMA MALO ");
              menSalida=" CLIMA:M";
              analogWrite(rojo,255);
              analogWrite(verde,0);
              analogWrite(azul,0);
             }
          }
          if(salida>=0.3 && salida<=0.7){
             if(salida>=0.41 && salida<=0.5){
              Serial.print(" - CLIMA NORMAL ");
              menSalida=" CLIMA:N";
              analogWrite(rojo,0);
              analogWrite(verde,255);
              analogWrite(azul,0);
             }else{
                if(salida>=0.5 && salida<=0.7){
                  Serial.print(" - CLIMA ENTRE NORMAL Y BUENO "); 
                  menSalida=" CLIMA:NyB";
                  analogWrite(rojo,0);
                  analogWrite(verde,255);
                  analogWrite(azul,255); 
                }
             }
          }
          if(salida>=0.5 && salida<=1){
             if(salida>=0.7 && salida<=1){
              Serial.print(" - CLIMA BUENO ");
              menSalida=" CLIMA:B";
              analogWrite(rojo,0);
              analogWrite(verde,0);
              analogWrite(azul,255);
             }
          }
          Serial.println("");
          delay(1000);
          //MOSTRAR EN EL LCD
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("T:");
          lcd.print(temperatura);
          lcd.print(" H:");
          lcd.print(humedad);
          lcd.setCursor(0,1);
          lcd.print("S:");
          lcd.print(salida);
          lcd.print(menSalida);
      }else{
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("FUERA DE RANGO");
      }
      
}

//***********************************************************************
// Funciones de soporte para el Sistema de Inferencia Difusa                        
//***********************************************************************
// Triangular Member Function
//FUNCION TRIANGULAR
FIS_TYPE fis_trimf(FIS_TYPE x, FIS_TYPE* p)
{
    FIS_TYPE a = p[0], b = p[1], c = p[2];
    FIS_TYPE t1 = (x - a) / (b - a);
    FIS_TYPE t2 = (c - x) / (c - b);
    if ((a == b) && (b == c)) return (FIS_TYPE) (x == a);
    if (a == b) return (FIS_TYPE) (t2*(b <= x)*(x <= c));
    if (b == c) return (FIS_TYPE) (t1*(a <= x)*(x <= b));
    t1 = min(t1, t2);
    return (FIS_TYPE) max(t1, 0);
}

//retorna minimos de la funciones de dependencia (Mamdani)
FIS_TYPE fis_min(FIS_TYPE a, FIS_TYPE b)
{
    return min(a, b);
}
//retorna maximos de la funciones de dependencia (Mamdani)
FIS_TYPE fis_max(FIS_TYPE a, FIS_TYPE b)
{
    return max(a, b);
}

FIS_TYPE fis_array_operation(FIS_TYPE *array, int size, _FIS_ARR_OP pfnOp)
{
    int i;
    FIS_TYPE ret = 0;
    if (size == 0) return ret;
    if (size == 1) return array[0];
    ret = array[0];
    for (i = 1; i < size; i++)
    {
        ret = (*pfnOp)(ret, array[i]);
    }
    return ret;
}

//***********************************************************************
// Datos para el sistema de inferencia difusa                                     
//***********************************************************************
//Punteros a las implementaciones de funciones de membresia
_FIS_MF fis_gMF[] =
{
    fis_trimf
};

// Recuento de función miembro para cada entrada
int fis_gIMFCount[] = { 4, 4 };

// Recuento de función miembro para cada salida
int fis_gOMFCount[] = { 3 };

// Coeficientes para las funciones miembro de entrada
FIS_TYPE fis_gMFI0Coeff1[] = { -5, 5, 10 }; //SE CAMBIA A -5
FIS_TYPE fis_gMFI0Coeff2[] = { 8, 14, 18 };
FIS_TYPE fis_gMFI0Coeff3[] = { 15, 20, 25 };
FIS_TYPE fis_gMFI0Coeff4[] = { 20, 25, 50 };//SE CAMBIO 42 POR 50
FIS_TYPE* fis_gMFI0Coeff[] = { fis_gMFI0Coeff1, fis_gMFI0Coeff2, fis_gMFI0Coeff3, fis_gMFI0Coeff4 };
FIS_TYPE fis_gMFI1Coeff1[] = { 0, 20, 40 };
FIS_TYPE fis_gMFI1Coeff2[] = { 50, 55, 60 };
FIS_TYPE fis_gMFI1Coeff3[] = { 65, 85, 100 };
FIS_TYPE fis_gMFI1Coeff4[] = { 35, 55, 70 };
FIS_TYPE* fis_gMFI1Coeff[] = { fis_gMFI1Coeff1, fis_gMFI1Coeff2, fis_gMFI1Coeff3, fis_gMFI1Coeff4 };
FIS_TYPE** fis_gMFICoeff[] = { fis_gMFI0Coeff, fis_gMFI1Coeff };

// Coeficientes para las funciones miembro de salida
FIS_TYPE fis_gMFO0Coeff1[] = { 0, 0.2, 0.4167 }; //primer triangular de salida (MALO)
FIS_TYPE fis_gMFO0Coeff2[] = { 0.3, 0.5, 0.7 }; //segunda triagular de salida (NORMAL)
FIS_TYPE fis_gMFO0Coeff3[] = { 0.5, 0.8, 1 }; //tercera triangular de salida (BUENO)
FIS_TYPE* fis_gMFO0Coeff[] = { fis_gMFO0Coeff1, fis_gMFO0Coeff2, fis_gMFO0Coeff3 };
FIS_TYPE** fis_gMFOCoeff[] = { fis_gMFO0Coeff };

//Conjunto de funciones de membresía de entrada
int fis_gMFI0[] = { 0, 0, 0, 0 };
int fis_gMFI1[] = { 0, 0, 0, 0 };
int* fis_gMFI[] = { fis_gMFI0, fis_gMFI1};

// Output membership function set
//Conjunto de funciones de membresía de salida
int fis_gMFO0[] = { 0, 0, 0 };
int* fis_gMFO[] = { fis_gMFO0};


//pesos de reglas
FIS_TYPE fis_gRWeight[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

// Tipo de regla
int fis_gRType[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

// REGLAS DE ENTRADA
int fis_gRI0[] = { 1, 1 };
int fis_gRI1[] = { 1, 4 };
int fis_gRI2[] = { 1, 2 };
int fis_gRI3[] = { 1, 3 };
int fis_gRI4[] = { 2, 1 };
int fis_gRI5[] = { 2, 4 };
int fis_gRI6[] = { 2, 2 };
int fis_gRI7[] = { 2, 3 };
int fis_gRI8[] = { 3, 1 };
int fis_gRI9[] = { 3, 4 };
int fis_gRI10[] = { 3, 2 };
int fis_gRI11[] = { 3, 3 };
int fis_gRI12[] = { 4, 1 };
int fis_gRI13[] = { 4, 4 };
int fis_gRI14[] = { 4, 2 };
int fis_gRI15[] = { 4, 3 };
int* fis_gRI[] = { fis_gRI0, fis_gRI1, fis_gRI2, fis_gRI3, fis_gRI4, fis_gRI5, fis_gRI6, fis_gRI7, fis_gRI8, fis_gRI9, fis_gRI10, fis_gRI11, fis_gRI12, fis_gRI13, fis_gRI14, fis_gRI15 };

// REGLAS DE SALIDA
int fis_gRO0[] = { 1 };
int fis_gRO1[] = { 2 };
int fis_gRO2[] = { 2 };
int fis_gRO3[] = { 1 };
int fis_gRO4[] = { 1 };
int fis_gRO5[] = { 2 };
int fis_gRO6[] = { 3 };
int fis_gRO7[] = { 1 };
int fis_gRO8[] = { 1 };
int fis_gRO9[] = { 3 };
int fis_gRO10[] = { 3 };
int fis_gRO11[] = { 1 };
int fis_gRO12[] = { 1 };
int fis_gRO13[] = { 3 };
int fis_gRO14[] = { 3 };
int fis_gRO15[] = { 1 };
int* fis_gRO[] = { fis_gRO0, fis_gRO1, fis_gRO2, fis_gRO3, fis_gRO4, fis_gRO5, fis_gRO6, fis_gRO7, fis_gRO8, fis_gRO9, fis_gRO10, fis_gRO11, fis_gRO12, fis_gRO13, fis_gRO14, fis_gRO15 };

// Rango minimo de entrada
FIS_TYPE fis_gIMin[] = { 0, 0 };

// Rango maximo de entrada

FIS_TYPE fis_gIMax[] = { 50, 100 };//50 GRADOS Y 100 HUMEDAD

// Rango minimo de salida
FIS_TYPE fis_gOMin[] = { 0 };

// Rango maximo de salida
FIS_TYPE fis_gOMax[] = { 1 };

//***********************************************************************
// Funciones de soporte dependientes de los datos para el sistema de inferencia difusa          
//***********************************************************************
FIS_TYPE fis_MF_out(FIS_TYPE** fuzzyRuleSet, FIS_TYPE x, int o)
{
    FIS_TYPE mfOut;
    int r;
    //MACHEAR LAS REGLAS CON LOS COEFICIENTES
    for (r = 0; r < fis_gcR; ++r)
    {
        int index = fis_gRO[r][o];
        if (index > 0)
        {
            index = index - 1;
            mfOut = (fis_gMF[fis_gMFO[o][index]])(x, fis_gMFOCoeff[o][index]);
        }
        else if (index < 0)
        {
            index = -index - 1;
            mfOut = 1 - (fis_gMF[fis_gMFO[o][index]])(x, fis_gMFOCoeff[o][index]);
        }
        else
        {
            mfOut = 0;
        }

        fuzzyRuleSet[0][r] = fis_min(mfOut, fuzzyRuleSet[1][r]);
    }
    return fis_array_operation(fuzzyRuleSet[0], fis_gcR, fis_max);
}

FIS_TYPE fis_defuzz_centroid(FIS_TYPE** fuzzyRuleSet, int o)
{
    FIS_TYPE step = (fis_gOMax[o] - fis_gOMin[o]) / (FIS_RESOLUSION - 1);
    FIS_TYPE area = 0;
    FIS_TYPE momentum = 0;
    FIS_TYPE dist, slice;
    int i;
    
    //calcular el área bajo la curva formada por las salidas MF
    for (i = 0; i < FIS_RESOLUSION; ++i){
        dist = fis_gOMin[o] + (step * i);
        slice = step * fis_MF_out(fuzzyRuleSet, dist, o);
        area += slice;
        momentum += slice*dist;
    }
    return ((area == 0) ? ((fis_gOMax[o] + fis_gOMin[o]) / 2) : (momentum / area));
}

//***********************************************************************
// Sistema de inferencia difusa                                               
//***********************************************************************
void fis_evaluate()
{
    FIS_TYPE fuzzyInput0[] = { 0, 0, 0, 0 };
    FIS_TYPE fuzzyInput1[] = { 0, 0, 0, 0 };
    FIS_TYPE* fuzzyInput[fis_gcI] = { fuzzyInput0, fuzzyInput1, };
    FIS_TYPE fuzzyOutput0[] = { 0, 0, 0 };
    FIS_TYPE* fuzzyOutput[fis_gcO] = { fuzzyOutput0, };
    FIS_TYPE fuzzyRules[fis_gcR] = { 0 };
    FIS_TYPE fuzzyFires[fis_gcR] = { 0 };
    FIS_TYPE* fuzzyRuleSet[] = { fuzzyRules, fuzzyFires };
    FIS_TYPE sW = 0;

    // Transformación de entrada a entrada difusa
    int i, j, r, o;
    for (i = 0; i < fis_gcI; ++i)
    {
        for (j = 0; j < fis_gIMFCount[i]; ++j)
        {
            fuzzyInput[i][j] =
                (fis_gMF[fis_gMFI[i][j]])(g_fisInput[i], fis_gMFICoeff[i][j]);
        }
    }

    int index = 0;
    for (r = 0; r < fis_gcR; ++r)
    {
        if (fis_gRType[r] == 1)
        {
            fuzzyFires[r] = FIS_MAX;
            for (i = 0; i < fis_gcI; ++i)
            {
                index = fis_gRI[r][i];
                if (index > 0)
                    fuzzyFires[r] = fis_min(fuzzyFires[r], fuzzyInput[i][index - 1]); //MINIMOS DE FUNCIONES DE PERTENENCIA
                else if (index < 0)
                    fuzzyFires[r] = fis_min(fuzzyFires[r], 1 - fuzzyInput[i][-index - 1]);
                else
                    fuzzyFires[r] = fis_min(fuzzyFires[r], 1);
            }
        }
        else
        {
            fuzzyFires[r] = FIS_MIN;
            for (i = 0; i < fis_gcI; ++i)
            {
                index = fis_gRI[r][i];
                if (index > 0)
                    fuzzyFires[r] = fis_max(fuzzyFires[r], fuzzyInput[i][index - 1]);
                else if (index < 0)
                    fuzzyFires[r] = fis_max(fuzzyFires[r], 1 - fuzzyInput[i][-index - 1]);
                else
                    fuzzyFires[r] = fis_max(fuzzyFires[r], 0);
            }
        }

        fuzzyFires[r] = fis_gRWeight[r] * fuzzyFires[r];
        sW += fuzzyFires[r];
    }

    if (sW == 0)
    {
        for (o = 0; o < fis_gcO; ++o)
        {
            g_fisOutput[o] = ((fis_gOMax[o] + fis_gOMin[o]) / 2);
        }
    }
    else
    {
        for (o = 0; o < fis_gcO; ++o)
        {
            g_fisOutput[o] = fis_defuzz_centroid(fuzzyRuleSet, o); //SE ENVIA EL SETEO DE REGLAS A LA FUNCION CENTROIDE
        }
    }
}
