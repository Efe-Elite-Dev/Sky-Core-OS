void kernel_main() {
    setup_graphics();
    init_mouse();
    
    SetupData my_setup = {0};
    int current_step = 0;

    while(1) {
        draw_setup_screen(current_step, &my_setup);
        
        // Eğer fare tıklandıysa ve "İleri" butonunun koordinatlarındaysa:
        if(is_mouse_clicked_on(420, 100)) {
            current_step++;
        }
    }
}
