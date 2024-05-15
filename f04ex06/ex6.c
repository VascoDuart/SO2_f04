#include <windows.h>
#include <tchar.h>
#include <math.h>

#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

#define MAX 6000



// funcionalidade relacionada com temporização

static double PerfCounterFreq; // n ticks por seg.

void initClock() {
	LARGE_INTEGER aux;
	if (!QueryPerformanceFrequency(&aux))
		_tprintf(TEXT("\nSorry - No can do em QueryPerfFreq\n"));
	PerfCounterFreq = (double)(aux.QuadPart); // / 1000.0;
	_tprintf(TEXT("\nTicks por sec.%f\n"), PerfCounterFreq);
}

__int64 startClock() {
	LARGE_INTEGER aux;
	QueryPerformanceCounter(&aux);
	//_tprintf_s(_T("Ticks = %lld\n"), aux.QuadPart);
	return aux.QuadPart;
}

double stopClock(__int64 from) {
	LARGE_INTEGER aux;
	QueryPerformanceCounter(&aux);
	_tprintf_s(_T("Ticks = %lld\n"), aux.QuadPart - from);
	return (double)(aux.QuadPart - from) / PerfCounterFreq;
}

// estrutura de dados para controlar as threads

typedef struct {
	unsigned int* pSoma;
	unsigned int* pNumBloco;
	CRITICAL_SECTION* pCs;
	BOOL* pContinua;
	HANDLE hEv;
} TDados;

#define TAM_BLOCO 10000
#define MAX_THREADS 20


// função da(s) thread(s)
DWORD WINAPI threadFunction(LPVOID data) {
	TDados* td = (TDados*)data;
	DWORD i = 0;
	unsigned int bloco, inicio, fim;
	BOOL cont;

	while (TRUE) {

		EnterCriticalSection(td->pCs);
		bloco = *(td->pNumBloco);
		(*(td->pNumBloco));
		cont = td->pContinua;
		LeaveCriticalSection(td->pCs);

		if (!cont)
			break;

		inicio = bloco * TAM_BLOCO + 1;
		fim = (bloco + 1) * TAM_BLOCO;

		_tprintf_s(_T("\nInicio: %u...%u\n"), inicio, fim);

		for (i = inicio; i <= fim; i++) {
			if (i % 3 == 0) {
				EnterCriticalSection(td->pCs, INFINITE);
				(*(td->pSoma))++;

				LeaveCriticalSection(td->pCs);
			}
		}
		SetEvent(td->hEv);

	}
	_tprintf_s(_T("\nFim!!!\n"));
	return 0;
}



DWORD WINAPI comandos(LPVOID data) {
	TDados* td = (TDados*)data;
	TCHAR str[40];
	DWORD i = 0;

	_tprintf_s(_T("COMANDO: "));

	do {
		_tscanf_s(_T("%s"), str, 40);
		if (_tcscmp(str, _T("exit")) == 0) {
			EnterCriticalSection(td->pCs);
			*(td->pContinua) = FALSE;
			LeaveCriticalSection(td->pCs);
			SetEvent(td->hEv);
			break;
		}

	} while (TRUE);
	return 0;
}
// ...

// número * máximo * de threads
// podem (e devem) ser menos
#define MAX_THREADS 20





int _tmain(int argc, TCHAR* argv[]) {

	CRITICAL_SECTION csGlobal;
	HANDLE hEvGlobal;

	// matriz de handles das threads
	HANDLE hThreads[MAX_THREADS];

	// Matriz de dados para as threads;
	TDados tdados[MAX_THREADS];

	// número efectivo de threads
	int numthreads;

	// limite superior
	unsigned int limsup;

	// variáveis para cronómetro
	__int64 clockticks;
	double duracao;

	unsigned int range;
	unsigned int inter;
	unsigned int soma = 0;
	unsigned int numBloco = 0;

	BOOL continua = TRUE;

	DWORD res;
	int i;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif 

	initClock();
	_tprintf_s(TEXT("\nNum threads -> "));
	_tscanf_s(TEXT("%u"), &numthreads);
	if (numthreads > MAX_THREADS)
		numthreads = MAX_THREADS;

	// FAZER prepara e cria threads
	//       manda as threads começar

	InitializeCriticalSectionAndSpinCount(&csGlobal, 300);

	HANDLE hEv = CreateEvent(NULL, FALSE, FALSE, NULL);		//evento reset automático -> primeiro false

	hEvGlobal = CreateEvent(NULL, TRUE, FALSE, NULL);

	for (i = 0; i < numthreads; i++) {
		tdados[i].pNumBloco = &numBloco;
		tdados[i].pSoma = &soma;
		tdados[i].pContinua = &continua;
		tdados[i].pCs = &csGlobal;
		tdados[i].hEv = hEv;

		hThreads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadFunction, (LPBYTE)&tdados[i], CREATE_SUSPENDED, NULL);

	}

	HANDLE hTh;
	TDados tinfo;

	tinfo.pNumBloco = &numBloco;
	tinfo.pSoma = &soma;
	tinfo.pContinua = &continua;
	tinfo.pCs = &csGlobal;
	tinfo.hEv = hEv;

	hTh = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)comandos, (LPBYTE)&tinfo, CREATE_SUSPENDED, NULL);

	_tprintf_s(_T("\nCarregue em qualquer tecla para começar...\n"));
	_gettch();

	for (i = 0; i < numthreads; i++) {
		ResumeThread(hThreads[i]);
	}

	clockticks = startClock();


	BOOL cont;

	do {					//Thread preto (Ti)
		WaitForSingleObject(hEv, INFINITE);		//EVENTO de reset automático...
		_tprintf_s(_T("*"));
		EnterCriticalSection(&csGlobal);
		cont = continua;
		LeaveCriticalSection(&csGlobal);
		if (cont)
			_tprintf_s(_T("*"));
	} while (cont);


	res = WaitForMultipleObjects(numthreads, hThreads, TRUE, INFINITE);
	if (res == WAIT_OBJECT_0) {
		duracao = stopClock(clockticks);
		_tprintf_s(_T("\nSegundos%f\n"), duracao);
		_tprintf_s(_T("\nSoma=%u\n"), soma);
		DeleteCriticalSection(&csGlobal);
	}

	// FAZER aguarda / controla as threads 
	//       manda as threads parar

	//Sleep(5000);

	duracao = stopClock(clockticks);
	_tprintf(TEXT("\nSegundos=%f\n"), duracao);

	// FAZER apresenta resultados

	// Cód. ref. para aguardar por uma tecla – caso faça falta
	// _tprintf(TEXT("\nCarregue numa tecla"));
	// _gettch();

	return 0;
}
// Este código é apenas uma ajuda para o exercício. Se quiser, mude-o