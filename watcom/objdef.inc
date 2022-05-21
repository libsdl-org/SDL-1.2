OBJS  = $(OUTDIR)/SDL.obj &
	$(OUTDIR)/SDL_getenv.obj &
	$(OUTDIR)/SDL_iconv.obj &
	$(OUTDIR)/SDL_malloc.obj &
	$(OUTDIR)/SDL_qsort.obj &
	$(OUTDIR)/SDL_string.obj &
	$(OUTDIR)/SDL_error.obj &
	$(OUTDIR)/SDL_audio.obj &
	$(OUTDIR)/SDL_audiodev.obj &
	$(OUTDIR)/SDL_diskaudio.obj &
	$(OUTDIR)/SDL_active.obj &
	$(OUTDIR)/SDL_audiocvt.obj &
	$(OUTDIR)/SDL_blit.obj &
	$(OUTDIR)/SDL_blit_0.obj &
	$(OUTDIR)/SDL_blit_1.obj &
	$(OUTDIR)/SDL_blit_A.obj &
	$(OUTDIR)/SDL_blit_N.obj &
	$(OUTDIR)/SDL_bmp.obj &
	$(OUTDIR)/SDL_cdrom.obj &
	$(OUTDIR)/SDL_cursor.obj &
	$(OUTDIR)/SDL_dibaudio.obj &
	$(OUTDIR)/SDL_dibevents.obj &
	$(OUTDIR)/SDL_dibvideo.obj &
	$(OUTDIR)/SDL_nullvideo.obj &
	$(OUTDIR)/SDL_nullevents.obj &
	$(OUTDIR)/SDL_dx5audio.obj &
	$(OUTDIR)/SDL_dx5events.obj &
	$(OUTDIR)/SDL_dx5video.obj &
	$(OUTDIR)/SDL_dx5yuv.obj &
	$(OUTDIR)/SDL_events.obj &
	$(OUTDIR)/SDL_expose.obj &
	$(OUTDIR)/SDL_fatal.obj &
	$(OUTDIR)/SDL_gamma.obj &
	$(OUTDIR)/SDL_joystick.obj &
	$(OUTDIR)/SDL_keyboard.obj &
	$(OUTDIR)/SDL_mixer.obj &
	$(OUTDIR)/SDL_mixer_MMX_VC.obj &
	$(OUTDIR)/SDL_mmjoystick.obj &
	$(OUTDIR)/SDL_mouse.obj &
	$(OUTDIR)/SDL_pixels.obj &
	$(OUTDIR)/SDL_quit.obj &
	$(OUTDIR)/SDL_resize.obj &
	$(OUTDIR)/SDL_RLEaccel.obj &
	$(OUTDIR)/SDL_rwops.obj &
	$(OUTDIR)/SDL_stretch.obj &
	$(OUTDIR)/SDL_surface.obj &
	$(OUTDIR)/SDL_sysloadso.obj &
	$(OUTDIR)/SDL_syscdrom.obj &
	$(OUTDIR)/SDL_syscond.obj &
	$(OUTDIR)/SDL_sysevents.obj &
	$(OUTDIR)/SDL_sysmouse.obj &
	$(OUTDIR)/SDL_sysmutex.obj &
	$(OUTDIR)/SDL_syssem.obj &
	$(OUTDIR)/SDL_systhread.obj &
	$(OUTDIR)/SDL_systimer.obj &
	$(OUTDIR)/SDL_syswm.obj &
	$(OUTDIR)/SDL_thread.obj &
	$(OUTDIR)/SDL_timer.obj &
	$(OUTDIR)/SDL_video.obj &
	$(OUTDIR)/SDL_wave.obj &
	$(OUTDIR)/SDL_wingl.obj &
	$(OUTDIR)/SDL_yuv.obj &
	$(OUTDIR)/SDL_yuv_sw.obj &
	$(OUTDIR)/SDL_yuv_mmx.obj &
	$(OUTDIR)/SDL_dummyaudio.obj &
	$(OUTDIR)/SDL_cpuinfo.obj

ASMOBJS=$(OUTDIR)/x86_main.obj &
	$(OUTDIR)/x86p_16.obj &
	$(OUTDIR)/x86p_32.obj &
	$(OUTDIR)/mmx_main.obj &
	$(OUTDIR)/mmxp2_32.obj

$(OUTDIR)/SDL.obj :           .AUTODEPEND $(SRCDIR)/SDL.c ; $(COMPILE_C)
$(OUTDIR)/SDL_getenv.obj :    .AUTODEPEND $(SRCDIR)/stdlib/SDL_getenv.c ; $(COMPILE_C)
$(OUTDIR)/SDL_iconv.obj :     .AUTODEPEND $(SRCDIR)/stdlib/SDL_iconv.c ; $(COMPILE_C)
$(OUTDIR)/SDL_malloc.obj :    .AUTODEPEND $(SRCDIR)/stdlib/SDL_malloc.c ; $(COMPILE_C)
$(OUTDIR)/SDL_qsort.obj :     .AUTODEPEND $(SRCDIR)/stdlib/SDL_qsort.c ; $(COMPILE_C)
$(OUTDIR)/SDL_string.obj :    .AUTODEPEND $(SRCDIR)/stdlib/SDL_string.c ; $(COMPILE_C)
$(OUTDIR)/SDL_active.obj :    .AUTODEPEND $(SRCDIR)/events/SDL_active.c ; $(COMPILE_C)
$(OUTDIR)/SDL_audio.obj :     .AUTODEPEND $(SRCDIR)/audio/SDL_audio.c ; $(COMPILE_C)
$(OUTDIR)/SDL_audiodev.obj :  .AUTODEPEND $(SRCDIR)/audio/SDL_audiodev.c ; $(COMPILE_C)
$(OUTDIR)/SDL_audiocvt.obj :  .AUTODEPEND $(SRCDIR)/audio/SDL_audiocvt.c ; $(COMPILE_C)
$(OUTDIR)/SDL_blit.obj :      .AUTODEPEND $(SRCDIR)/video/SDL_blit.c ; $(COMPILE_C)
$(OUTDIR)/SDL_blit_0.obj :    .AUTODEPEND $(SRCDIR)/video/SDL_blit_0.c ; $(COMPILE_C)
$(OUTDIR)/SDL_blit_1.obj :    .AUTODEPEND $(SRCDIR)/video/SDL_blit_1.c ; $(COMPILE_C)
$(OUTDIR)/SDL_blit_A.obj :    .AUTODEPEND $(SRCDIR)/video/SDL_blit_A.c ; $(COMPILE_C)
$(OUTDIR)/SDL_blit_N.obj :    .AUTODEPEND $(SRCDIR)/video/SDL_blit_N.c ; $(COMPILE_C)
$(OUTDIR)/SDL_bmp.obj :       .AUTODEPEND $(SRCDIR)/video/SDL_bmp.c ; $(COMPILE_C)
$(OUTDIR)/SDL_cdrom.obj :     .AUTODEPEND $(SRCDIR)/cdrom/SDL_cdrom.c ; $(COMPILE_C)
$(OUTDIR)/SDL_cursor.obj :    .AUTODEPEND $(SRCDIR)/video/SDL_cursor.c ; $(COMPILE_C)
$(OUTDIR)/SDL_dibaudio.obj :  .AUTODEPEND $(SRCDIR)/audio/windib/SDL_dibaudio.c ; $(COMPILE_C)
$(OUTDIR)/SDL_dibevents.obj : .AUTODEPEND $(SRCDIR)/video/windib/SDL_dibevents.c ; $(COMPILE_C)
$(OUTDIR)/SDL_dibvideo.obj :  .AUTODEPEND $(SRCDIR)/video/windib/SDL_dibvideo.c ; $(COMPILE_C)
$(OUTDIR)/SDL_nullvideo.obj : .AUTODEPEND $(SRCDIR)/video/dummy/SDL_nullvideo.c ; $(COMPILE_C)
$(OUTDIR)/SDL_dx5audio.obj :  .AUTODEPEND $(SRCDIR)/audio/windx5/SDL_dx5audio.c ; $(COMPILE_C)
$(OUTDIR)/SDL_dx5events.obj:  .AUTODEPEND $(SRCDIR)/video/windx5/SDL_dx5events.c ; $(COMPILE_C) -DINITGUID
$(OUTDIR)/SDL_dx5video.obj :  .AUTODEPEND $(SRCDIR)/video/windx5/SDL_dx5video.c ; $(COMPILE_C)
$(OUTDIR)/SDL_dx5yuv.obj :    .AUTODEPEND $(SRCDIR)/video/windx5/SDL_dx5yuv.c ; $(COMPILE_C)
$(OUTDIR)/SDL_error.obj :     .AUTODEPEND $(SRCDIR)/SDL_error.c ; $(COMPILE_C)
$(OUTDIR)/SDL_events.obj :    .AUTODEPEND $(SRCDIR)/events/SDL_events.c ; $(COMPILE_C)
$(OUTDIR)/SDL_expose.obj :    .AUTODEPEND $(SRCDIR)/events/SDL_expose.c ; $(COMPILE_C)
$(OUTDIR)/SDL_fatal.obj :     .AUTODEPEND $(SRCDIR)/SDL_fatal.c ; $(COMPILE_C)
$(OUTDIR)/SDL_gamma.obj :     .AUTODEPEND $(SRCDIR)/video/SDL_gamma.c ; $(COMPILE_C)
$(OUTDIR)/SDL_joystick.obj :  .AUTODEPEND $(SRCDIR)/joystick/SDL_joystick.c ; $(COMPILE_C)
$(OUTDIR)/SDL_keyboard.obj :  .AUTODEPEND $(SRCDIR)/events/SDL_keyboard.c ; $(COMPILE_C)
$(OUTDIR)/SDL_mixer.obj :     .AUTODEPEND $(SRCDIR)/audio/SDL_mixer.c ; $(COMPILE_C)
$(OUTDIR)/SDL_mmjoystick.obj: .AUTODEPEND $(SRCDIR)/joystick/win32/SDL_mmjoystick.c ; $(COMPILE_C)
$(OUTDIR)/SDL_mouse.obj :     .AUTODEPEND $(SRCDIR)/events/SDL_mouse.c ; $(COMPILE_C)
$(OUTDIR)/SDL_pixels.obj :    .AUTODEPEND $(SRCDIR)/video/SDL_pixels.c ; $(COMPILE_C)
$(OUTDIR)/SDL_quit.obj :      .AUTODEPEND $(SRCDIR)/events/SDL_quit.c ; $(COMPILE_C)
$(OUTDIR)/SDL_resize.obj :    .AUTODEPEND $(SRCDIR)/events/SDL_resize.c ; $(COMPILE_C)
$(OUTDIR)/SDL_RLEaccel.obj :  .AUTODEPEND $(SRCDIR)/video/SDL_RLEaccel.c ; $(COMPILE_C)
$(OUTDIR)/SDL_rwops.obj :     .AUTODEPEND $(SRCDIR)/file/SDL_rwops.c ; $(COMPILE_C)
$(OUTDIR)/SDL_stretch.obj :   .AUTODEPEND $(SRCDIR)/video/SDL_stretch.c ; $(COMPILE_C)
$(OUTDIR)/SDL_surface.obj :   .AUTODEPEND $(SRCDIR)/video/SDL_surface.c ; $(COMPILE_C)
$(OUTDIR)/SDL_syscdrom.obj :  .AUTODEPEND $(SRCDIR)/cdrom/win32/SDL_syscdrom.c ; $(COMPILE_C)
$(OUTDIR)/SDL_syscond.obj :   .AUTODEPEND $(SRCDIR)/thread/generic/SDL_syscond.c ; $(COMPILE_C)
$(OUTDIR)/SDL_sysevents.obj : .AUTODEPEND $(SRCDIR)/video/wincommon/SDL_sysevents.c ; $(COMPILE_C)
$(OUTDIR)/SDL_sysloadso.obj : .AUTODEPEND $(SRCDIR)/loadso/win32/SDL_sysloadso.c ; $(COMPILE_C)
$(OUTDIR)/SDL_sysmouse.obj :  .AUTODEPEND $(SRCDIR)/video/wincommon/SDL_sysmouse.c ; $(COMPILE_C)
$(OUTDIR)/SDL_sysmutex.obj :  .AUTODEPEND $(SRCDIR)/thread/win32/SDL_sysmutex.c ; $(COMPILE_C)
$(OUTDIR)/SDL_syssem.obj :    .AUTODEPEND $(SRCDIR)/thread/win32/SDL_syssem.c ; $(COMPILE_C)
$(OUTDIR)/SDL_systhread.obj:  .AUTODEPEND $(SRCDIR)/thread/win32/SDL_systhread.c ; $(COMPILE_C)
$(OUTDIR)/SDL_systimer.obj :  .AUTODEPEND $(SRCDIR)/timer/win32/SDL_systimer.c ; $(COMPILE_C)
$(OUTDIR)/SDL_syswm.obj :     .AUTODEPEND $(SRCDIR)/video/wincommon/SDL_syswm.c ; $(COMPILE_C)
$(OUTDIR)/SDL_thread.obj :    .AUTODEPEND $(SRCDIR)/thread/SDL_thread.c ; $(COMPILE_C)
$(OUTDIR)/SDL_timer.obj :     .AUTODEPEND $(SRCDIR)/timer/SDL_timer.c ; $(COMPILE_C)
$(OUTDIR)/SDL_video.obj :     .AUTODEPEND $(SRCDIR)/video/SDL_video.c ; $(COMPILE_C)
$(OUTDIR)/SDL_wave.obj :      .AUTODEPEND $(SRCDIR)/audio/SDL_wave.c ; $(COMPILE_C)
$(OUTDIR)/SDL_wingl.obj :     .AUTODEPEND $(SRCDIR)/video/wincommon/SDL_wingl.c ; $(COMPILE_C)
$(OUTDIR)/SDL_yuv.obj :       .AUTODEPEND $(SRCDIR)/video/SDL_yuv.c ; $(COMPILE_C)
$(OUTDIR)/SDL_yuv_sw.obj :    .AUTODEPEND $(SRCDIR)/video/SDL_yuv_sw.c ; $(COMPILE_C)
$(OUTDIR)/SDL_yuv_mmx.obj :   .AUTODEPEND $(SRCDIR)/video/SDL_yuv_mmx.c ; $(COMPILE_C)
$(OUTDIR)/SDL_cpuinfo.obj :   .AUTODEPEND $(SRCDIR)/cpuinfo/SDL_cpuinfo.c ; $(COMPILE_C)
$(OUTDIR)/SDL_diskaudio.obj : .AUTODEPEND $(SRCDIR)/audio/disk/SDL_diskaudio.c ; $(COMPILE_C)
$(OUTDIR)/SDL_dummyaudio.obj : .AUTODEPEND $(SRCDIR)/audio/dummy/SDL_dummyaudio.c ; $(COMPILE_C)
$(OUTDIR)/SDL_nullevents.obj : .AUTODEPEND $(SRCDIR)/video/dummy/SDL_nullevents.c ; $(COMPILE_C)
$(OUTDIR)/SDL_mixer_MMX_VC.obj : .AUTODEPEND $(SRCDIR)/audio/SDL_mixer_MMX_VC.c ; $(COMPILE_C)
$(OUTDIR)/x86_main.obj : .AUTODEPEND $(SRCDIR)/hermes/x86_main.asm ; $(COMPILE_ASM)
$(OUTDIR)/x86p_16.obj : .AUTODEPEND $(SRCDIR)/hermes/x86p_16.asm ; $(COMPILE_ASM)
$(OUTDIR)/x86p_32.obj : .AUTODEPEND $(SRCDIR)/hermes/x86p_32.asm ; $(COMPILE_ASM)
$(OUTDIR)/mmx_main.obj : .AUTODEPEND $(SRCDIR)/hermes/mmx_main.asm ; $(COMPILE_ASM)
$(OUTDIR)/mmxp2_32.obj : .AUTODEPEND $(SRCDIR)/hermes/mmxp2_32.asm ; $(COMPILE_ASM)
