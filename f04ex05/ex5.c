#include <windows.h>
#include <tchar.h>
#include <math.h>

#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

#define MAX 6000

// funcionalidade relacionada com temporiza��o

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
	unsigned int inicio;
	unsigned int fim;
	unsigned int* pSoma;
	CRITICAL_SECTION* pCs;
	HANDLE hEv;
} TDados;

// fun��o da(s) thread(s)
DWORD WINAPI threadFunction(LPVOID data) {
	TDados* td = (TDados*)data;
	DWORD i = 0;

	

	_tprintf_s(_T("\nInicio: %u...%u\n"), td->inicio, td->fim);
	WaitForSingleObject(td->hEv, INFINITE);
	for (i = td->inicio; i <= td->fim; i++) {
		if (i % 3 == 0) {
			EnterCriticalSection(td->pCs, INFINITE);
			(*(td->pSoma))++;

			LeaveCriticalSection(td->pCs);
		}
	}

	_tprintf_s(_T("\nFim!!!\n"));
	return 0;
}
// ...

// n�mero * m�ximo * de threads
// podem (e devem) ser menos
#define MAX_THREADS 20





int _tmain(int argc, TCHAR* argv[]) {

	CRITICAL_SECTION csGlobal;
	HANDLE hEvGlobal;

	// matriz de handles das threads
	HANDLE hThreads[MAX_THREADS];

	// Matriz de dados para as threads;
	TDados tdados[MAX_THREADS];

	// n�mero efectivo de threads
	int numthreads;

	// limite superior
	unsigned int limsup;

	// vari�veis para cron�metro
	__int64 clockticks;
	double duracao;

	unsigned int range;
	unsigned int inter;
	unsigned int soma = 0;
	DWORD res;
	int i;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif 

	initClock();
	_tprintf_s(TEXT("\nLimite sup. -> "));
	_tscanf_s(TEXT("%u"), &limsup);
	_tprintf_s(TEXT("\nNum threads -> "));
	_tscanf_s(TEXT("%u"), &numthreads);
	if (numthreads > MAX_THREADS)
		numthreads = MAX_THREADS;

	// FAZER prepara e cria threads
	//       manda as threads come�ar

	InitializeCriticalSectionAndSpinCount(&csGlobal, 300);

	hEvGlobal = CreateEvent(NULL, TRUE, FALSE, NULL);

	for (i = 0; i < numthreads; i++) {
		tdados[i].inicio = i * (limsup / numthreads) + 1;
		tdados[i].fim = (i + 1) * (limsup / numthreads);
		tdados[i].pSoma = &soma;
		tdados[i].pCs = &csGlobal;
		tdados[i].hEv = hEvGlobal;

		hThreads[i] = CreateThread(NULL, 0, threadFunction, &tdados[i], 0, NULL);

	}

	_tprintf_s(_T("\nCarregue em qualquer tecla para come�ar...\n"));
	_gettch();

	SetEvent(hEvGlobal);
	
	clockticks = startClock();

	

	res = WaitForMultipleObjects(numthreads, hThreads, TRUE, INFINITE);
	if (res == WAIT_OBJECT_0) {
		duracao = stopClock(clockticks);
		_tprintf_s(_T("\nSegundos%f\n"), duracao);
		_tprintf_s(_T("\nSoma=%u\n"), soma);
		DeleteCriticalSection(&csGlobal);
		CloseHandle(hEvGlobal);
	}

	// FAZER aguarda / controla as threads 
	//       manda as threads parar

	//Sleep(5000);

	duracao = stopClock(clockticks);
	_tprintf(TEXT("\nSegundos=%f\n"), duracao);

	// FAZER apresenta resultados

	// C�d. ref. para aguardar por uma tecla � caso fa�a falta
	// _tprintf(TEXT("\nCarregue numa tecla"));
	// _gettch();

	return 0;
}
// Este c�digo � apenas uma ajuda para o exerc�cio. Se quiser, mude-o