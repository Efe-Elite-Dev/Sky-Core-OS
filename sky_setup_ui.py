import customtkinter as ctk
from PIL import Image
import os

class SkyCoreSetup(ctk.CTk):
    def __init__(self, on_complete_callback=None):
        super().__init__()
        
        # Sanal makine ekran standartlarına tam uyum (1024x768)
        self.title("Sky Core OS v1.5 - Setup Wizard")
        self.geometry("1024x768")
        self.resizable(False, False)
        
        # Kurulum bitince ana işletim sistemini tetikleyecek köprü fonksiyonu
        self.on_setup_complete = on_complete_callback
        self.current_step = 1
        
        # Ana Konteyner (Sky Core temasına uygun derin gece moru)
        self.main_frame = ctk.CTkFrame(self, fg_color="#1a122c")
        self.main_frame.pack(fill="both", expand=True)
        
        # Tasarladığın resimleri (pusula.png, 33.png, firtina.png) güvenli yükle
        self.load_assets()
        
        # İlk adımı çizerek başla
        self.show_step()

    def load_assets(self):
        """ Tasarladığın imajları projenin asset havuzuna dahil eder """
        self.assets = {}
        image_files = {
            "pusula": "pusula.png",
            "ay": "33.png",
            "firtina": "firtina.png"
        }
        for key, filename in image_files.items():
            if os.path.exists(filename):
                try:
                    self.assets[key] = ctk.CTkImage(light_image=Image.open(filename), dark_image=Image.open(filename))
                except:
                    self.assets[key] = None
            else:
                self.assets[key] = None

    def clear_frame(self):
        for widget in self.main_frame.winfo_children():
            widget.destroy()

    def show_step(self):
        self.clear_frame()
        if self.current_step == 1:
            self.render_step_1()
        elif self.current_step == 2:
            self.render_step_2()
        elif self.current_step == 3:
            self.render_step_3()
        elif self.current_step == 4:
            self.render_step_4()

    # --- 1. EKRAN: HOŞ GELDİNİZ ---
    def render_step_1(self):
        title_lbl = ctk.CTkLabel(self.main_frame, text="İLK KURULUM EKRANI - HOŞ GELDİNİZ", font=("Arial", 26, "bold"), text_color="white")
        title_lbl.pack(pady=25)
        
        center_card = ctk.CTkFrame(self.main_frame, fg_color="#241a3e", corner_radius=15, width=650, height=450)
        center_card.pack_propagate(False)
        center_card.pack(pady=30)
        
        # Pusula İkonu Kontrolü
        if self.assets.get("pusula"):
            self.assets["pusula"].configure(size=(100, 100))
            logo_img = ctk.CTkLabel(center_card, image=self.assets["pusula"], text="")
            logo_img.pack(pady=30)
        else:
            ctk.CTkLabel(center_card, text="🧭", font=("Arial", 60)).pack(pady=30)
            
        msg_lbl = ctk.CTkLabel(center_card, text="SKY CORE OS\nSisteme Hoş Geldiniz!", text_color="white", font=("Arial", 24, "bold"))
        msg_lbl.pack(pady=20)
        
        btn_frame = ctk.CTkFrame(center_card, fg_color="transparent")
        btn_frame.pack(side="bottom", pady=40)
        
        fast_btn = ctk.CTkButton(btn_frame, text="Hızlı Kurulum (Az)", fg_color="#2b3a67", hover_color="#3a4f8c", width=180, height=40, font=("Arial", 14, "bold"), command=self.next_step)
        fast_btn.pack(side="left", padx=15)
        
        detailed_btn = ctk.CTkButton(btn_frame, text="Detaylı Kurulum (Başka)", fg_color="#44356a", hover_color="#5f5380", width=180, height=40, font=("Arial", 14, "bold"), command=self.next_step)
        detailed_btn.pack(side="left", padx=15)

    # --- 2. EKRAN: KONUM & SAAT AYARLAMA ---
    def render_step_2(self):
        title_lbl = ctk.CTkLabel(self.main_frame, text="KONUM & SAAT AYARLAMA", font=("Arial", 24, "bold"), text_color="white")
        title_lbl.pack(pady=15)
        
        # Sol Panel (Teşekkür Mesajı ve Wi-Fi)
        left_panel = ctk.CTkFrame(self.main_frame, fg_color="#241a3e", width=460, height=540)
        left_panel.pack_propagate(False)
        left_panel.pack(side="left", padx=25, pady=10)
        
        thanks_lbl = ctk.CTkLabel(left_panel, text="SKY CORE OS v1.5'i\nSeçtiğiniz İçin\nTeşekkürler!", font=("Arial", 22, "bold"), text_color="#00d2d3")
        thanks_lbl.pack(pady=25)
        
        wifi_title = ctk.CTkLabel(left_panel, text="📶 WiFi A BAĞLANA", font=("Arial", 16, "bold"), text_color="white")
        wifi_title.pack(pady=10)
        
        wifi_box = ctk.CTkFrame(left_panel, fg_color="#1a122c", width=400, height=140)
        wifi_box.pack_propagate(False)
        wifi_box.pack(pady=5)
        
        ctk.CTkLabel(wifi_box, text="📶 Ağ Adı           ● Mükemmel Güç", text_color="#1dd1a1", font=("Arial", 13)).pack(pady=15)
        ctk.CTkLabel(wifi_box, text="📶 Ağ Adı 2         ● Bağlantı Yok", text_color="gray", font=("Arial", 13)).pack(pady=5)
        
        skip_btn = ctk.CTkButton(left_panel, text="Atla", fg_color="#44356a", width=150, height=35, command=self.next_step)
        skip_btn.pack(side="bottom", pady=25)

        # Sağ Panel (Harita ve Kaydetme)
        right_panel = ctk.CTkFrame(self.main_frame, fg_color="#241a3e", width=460, height=540)
        right_panel.pack_propagate(False)
        right_panel.pack(side="right", padx=25, pady=10)
        
        map_box = ctk.CTkFrame(right_panel, fg_color="#362958", width=410, height=220)
        map_box.pack_propagate(False)
        map_box.pack(pady=20)
        ctk.CTkLabel(map_box, text="🗺️ [ TÜRKİYE HARİTASI AKTİF ]", font=("Arial", 16, "bold"), text_color="white").place(relx=0.5, rely=0.5, anchor="center")
        
        ctk.CTkLabel(right_panel, text="Konum: [Istanbul, Türkiye]", font=("Arial", 15), text_color="white").pack(pady=5)
        ctk.CTkLabel(right_panel, text="Bölge Saati: [GMT+03:00]", font=("Arial", 15), text_color="white").pack(pady=5)
        
        save_btn = ctk.CTkButton(right_panel, text="Konum ve Saat Ayarlarını Kaydet", fg_color="#2b3a67", font=("Arial", 14, "bold"), width=340, height=45, command=self.next_step)
        save_btn.pack(side="bottom", pady=35)

    # --- 3. EKRAN: GİRİŞ & TAMAMLAMA ---
    def render_step_3(self):
        title_lbl = ctk.CTkLabel(self.main_frame, text="GİRİŞ & TAMAMLAMA", font=("Arial", 24, "bold"), text_color="white")
        title_lbl.pack(pady=20)
        
        center_panel = ctk.CTkFrame(self.main_frame, fg_color="#241a3e", width=520, height=520)
        center_panel.pack_propagate(False)
        center_panel.pack(pady=20)
        
        ctk.CTkLabel(center_panel, text="Giriş Bilgilerini Kontrol Edin.", font=("Arial", 18), text_color="white").pack(pady=20)
        
        # Ay Logosu (33.png) Kontrolü
        if self.assets.get("ay"):
            self.assets["ay"].configure(size=(120, 120))
            moon_img = ctk.CTkLabel(center_panel, image=self.assets["ay"], text="")
            moon_img.pack(pady=20)
        else:
            ctk.CTkLabel(center_panel, text="🌙\n[33.png]", font=("Arial", 20), text_color="#a29bfe").pack(pady=20)
            
        ctk.CTkLabel(center_panel, text="Teşekkürler, Kullanıma Hazır!\nMasaüstüne Gitmek İçin\nHAZIR", font=("Arial", 18, "bold"), text_color="white").pack(pady=15)
        
        start_btn = ctk.CTkButton(center_panel, text="BAŞLAT", fg_color="#10ac84", hover_color="#1dd1a1", font=("Arial", 16, "bold"), width=280, height=45, command=self.next_step)
        start_btn.pack(side="bottom", pady=35)

    # --- 4. EKRAN: TASARLADIĞIN ÇEKMECELİ MASAÜSTÜ ---
    def render_step_4(self):
        self.main_frame.rowconfigure(0, weight=1)
        self.main_frame.rowconfigure(1, weight=0)
        self.main_frame.columnconfigure(0, weight=1)
        
        # Şemadaki geniş fırtınalı masaüstü alanı
        desktop_area = ctk.CTkFrame(self.main_frame, fg_color="#1e1b29")
        desktop_area.grid(row=0, column=0, sticky="nsew")
        
        # Hava Durumu ve Saat Widget'ı (Tam ortada üstte)
        weather_widget = ctk.CTkFrame(desktop_area, fg_color="#2d2640", corner_radius=12, width=450, height=110)
        weather_widget.place(relx=0.5, rely=0.15, anchor="center")
        weather_lbl = ctk.CTkLabel(weather_widget, text="🕒 26:03\nİstanbul, 18°C Çok Bulutlu", font=("Arial", 14, "bold"), text_color="white")
        weather_lbl.place(relx=0.5, rely=0.5, anchor="center")
        
        # Sol sütun uygulamaları
        left_box = ctk.CTkLabel(desktop_area, text="📷 Kamera\n🎨 Galeri\n🎵 Müzik", text_color="#a29bfe", font=("Arial", 13), justify="left")
        left_box.place(x=40, rely=0.4)
        
        # Sağ sütun uygulamaları
        right_box = ctk.CTkLabel(desktop_area, text="💬 Mesajlar\n🖥️ Terminal\n📁 Dosyalar\n🗺️ Haritalar", text_color="#a29bfe", font=("Arial", 13), justify="right")
        right_box.place(relx=0.95, rely=0.4, anchor="ne")

        # --- ORTADAN ÇIKAN UYGULAMA ÇEKMECESİ ---
        self.app_drawer = ctk.CTkFrame(desktop_area, fg_color="#2d2942", border_color="#5f5380", border_width=2, corner_radius=15, width=600, height=380)
        self.app_drawer.place(relx=0.5, rely=0.58, anchor="center")
        
        search_bar = ctk.CTkEntry(self.app_drawer, placeholder_text="🔍 Uygulama adı veya terminal komutu girin...", width=520, fg_color="#1e1b29")
        search_bar.pack(pady=15)
        
        apps_frame = ctk.CTkFrame(self.app_drawer, fg_color="transparent")
        apps_frame.pack(fill="both", expand=True, padx=20, pady=5)
        
        # Grid biçiminde şemadaki alt kare uygulamalar
        items = ["Dosya Yöneticisi", "Takvim", "E-Posta", "Tarayıcı", "Kod Düzenleyici", "Sistem Ayarları"]
        for i, name in enumerate(items):
            r, c = i // 3, i % 3
            btn = ctk.CTkButton(apps_frame, text=f"📦\n{name}", fg_color="#3d3659", hover_color="#4d4470", width=150, height=80, font=("Arial", 12))
            btn.grid(row=r, column=c, padx=15, pady=15)

        # --- ALT GÖREV ÇUBUĞU (TASKBAR) ---
        taskbar = ctk.CTkFrame(self.main_frame, fg_color="#14111f", height=65, corner_radius=0)
        taskbar.grid(row=1, column=0, sticky="ew")
        taskbar.grid_propagate(False)
        
        # Sistem Simgeleri (Sol)
        sys_icons = ctk.CTkLabel(taskbar, text="🛠️ ⚙️ ℹ️ >_", text_color="white", font=("Arial", 16))
        sys_icons.pack(side="left", padx=30)
        
        # Ortadaki Tetikleyici Logo Butonu
        self.center_trigger_btn = ctk.CTkButton(
            taskbar, 
            text="🌪️ SKY CORE LOGO", 
            fg_color="#3d3659", 
            hover_color="#5f5380",
            width=180, 
            height=45,
            font=("Arial", 12, "bold"),
            command=self.toggle_drawer
        )
        self.center_trigger_btn.pack(relx=0.5, rely=0.5, anchor="center")
        
        # Sistemi tamamen başlatan sağ buton (CRITICAL FIX: Döngüyü kıran çıkış kapısı)
        launch_btn = ctk.CTkButton(taskbar, text="Sistemi Başlat 🚀", fg_color="#10ac84", hover_color="#1dd1a1", width=140, height=40, font=("Arial", 12, "bold"), command=self.finish_setup)
        launch_btn.pack(side="right", padx=20)

    def toggle_drawer(self):
        if self.app_drawer.winfo_viewable():
            self.app_drawer.place_forget()
        else:
            self.app_drawer.place(relx=0.5, rely=0.58, anchor="center")

    def next_step(self):
        if self.current_step < 4:
            self.current_step += 1
            self.show_step()

    def finish_setup(self):
        # Arka plandaki mainloop'u güvenli şekilde sonlandırıp kapatıyoruz
        self.quit()
        self.destroy()
