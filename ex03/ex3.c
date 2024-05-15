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
	unsigned int inicio;
	unsigned int fim;
	unsigned int* pSoma;
	HANDLE hMutex;
} TDados;

// função da(s) thread(s)
DWORD WINAPI conta(LPVOID data) {
	TDados* td = (TDados*)data;
	DWORD i = 0;

	_tprintf_s(_T("\nInicio: %u...%u\n"), td->inicio, td->fim);
	for (i = td->inicio; i <= td->fim; i++) {
		if (i % 3 == 0) {
			WaitForSingleObject(td->hMutex, INFINITE);
			(*(td->pSoma))++;

			ReleaseMutex(td->hMutex);
		}
	}

	_tprintf_s(_T("\nFim!!!\n"));
	return 0;
}
// ...

// número * máximo * de threads
// podem (e devem) ser menos
#define MAX_THREADS 20





int _tmain(int argc, TCHAR* argv[]) {

	HANDLE hMutexGlobal;

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
	//       manda as threads começar

	hMutexGlobal = CreateMutex(NULL, FALSE, NULL);

	for (i = 0; i < numthreads; i++) {
		tdados[i].inicio = i * (limsup / numthreads) + 1;
		tdados[i].fim = (i + 1) * (limsup / numthreads);
		tdados[i].pSoma = &soma;
		tdados[i].hMutex = hMutexGlobal;

		hThreads[i] = CreateThread(NULL, 0, conta, &tdados[i], CREATE_SUSPENDED, NULL);
	}

	_tprintf_s(_T("\nCarregue em qualquer tecla para começar...\n"));
	_gettch();

	for (i = 0; i < numthreads; i++) {
		ResumeThread(hThreads[i]);
	}
	clockticks = startClock();

	res = WaitForMultipleObjects(numthreads, hThreads, TRUE, INFINITE);
	if (res == WAIT_OBJECT_0) {
		duracao = stopClock(clockticks);
		_tprintf_s(_T("\nSegundos%f\n"), duracao);
		_tprintf_s(_T("\nSoma=%u\n"), soma);
		CloseHandle(hMutexGlobal);
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