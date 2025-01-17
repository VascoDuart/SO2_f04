#include <windows.h>
#include <tchar.h>
#include <math.h>

#include <stdio.h>
#include <fcntl.h> 
#include <io.h>


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
	// ...
	int x; // remover este inteiro. Est� aqui apenas para este c�digo compilar
} TDados;

// fun��o da(s) thread(s)
// ...

// n�mero * m�ximo * de threads
// podem (e devem) ser menos
#define MAX_THREADS 20





int _tmain(int argc, TCHAR* argv[]) {

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

	clockticks = startClock();

	// FAZER aguarda / controla as threads 
	//       manda as threads parar

	Sleep(5000);

	duracao = stopClock(clockticks);
	_tprintf(TEXT("\nSegundos=%f\n"), duracao);

	// FAZER apresenta resultados

	// C�d. ref. para aguardar por uma tecla � caso fa�a falta
	// _tprintf(TEXT("\nCarregue numa tecla"));
	// _gettch();

	return 0;
}
// Este c�digo � apenas uma ajuda para o exerc�cio. Se quiser, mude-o