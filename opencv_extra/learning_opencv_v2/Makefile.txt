TOP      := .

BINARIES :=  \
	ch2_ex2_1  \
	ch2_ex2_2  \
	ch2_ex2_3  \
	ch2_ex2_4  \
	ch2_ex2_5  \
	ch2_ex2_6  \
	ch2_ex2_7  \
	ch2_ex2_8  \
	ch2_ex2_9  \
	ch2_ex2_10 \
	ch3_ex3_3  \
	ch3_ex3_4  \
	ch3_ex3_5  \
	ch3_ex3_9  \
	ch3_ex3_11 \
	ch3_ex3_12 \
	ch3_ex3_13 \
	ch3_ex3_14 \
	ch3_ex3_15 \
	ch3_ex3_17 \
	ch3_ex3_19 \
	ch3_ex3_20 \
	ch4_ex4_1  \
	ch4_ex4_2  \
	ch4_ex4_3  \
	ch5_ex5_1  \
	ch5_ex5_2  \
	ch5_ex5_3  \
	ch5_ex5_4  \
	ch6_ex6_1  \
	ch6_ex6_2  \
	ch6_ex6_3  \
	ch6_ex6_4  \
	ch6_ex6_5  \
	ch7_ex7_1  \
	ch7_ex7_3_expanded \
	ch7_ex7_5_HistBackProj \
	ch7_ex7_5  \
	ch8_ex8_2  \
	ch8_ex8_3  \
	ch9_ex9_1    \
	ch9_background \
	ch9_watershed \
	ch10_ex10_1 \
	ch10_ex10_1b_Horn_Schunck  \
	ch10_motempl \
	ch10_ex10_2  \
	ch11_ex11_1  \
	ch11_ex11_1_fromdisk \
	ch12_ex12_1   \
	ch12_ex12_2  \
	ch12_ex12_3  \
	ch12_ex12_4  \
	ch13_ex13_1 \
	ch13_ex13_2 \
	ch13_ex13_3 \
	ch13_ex13_4



SUBDIRS  := 

FLAGS =  -I/usr/local/include/opencv # -Wall

include $(TOP)/common.mk

ch2_ex2_1: ch2_ex2_1.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch2_ex2_2: ch2_ex2_2.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch2_ex2_3: ch2_ex2_3.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch2_ex2_4: ch2_ex2_4.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch2_ex2_5: ch2_ex2_5.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch2_ex2_6: ch2_ex2_6.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch2_ex2_7: ch2_ex2_7.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch2_ex2_8: ch2_ex2_8.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch2_ex2_9: ch2_ex2_9.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch2_ex2_10: ch2_ex2_10.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch3_ex3_3: ch3_ex3_3.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch3_ex3_4: ch3_ex3_4.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch3_ex3_5: ch3_ex3_5.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch3_ex3_9: ch3_ex3_9.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch3_ex3_11: ch3_ex3_11.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch3_ex3_12: ch3_ex3_12.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch3_ex3_13: ch3_ex3_13.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch3_ex3_14: ch3_ex3_14.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch3_ex3_15: ch3_ex3_15.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch3_ex3_17: ch3_ex3_17.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch3_ex3_19: ch3_ex3_19.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch3_ex3_20: ch3_ex3_20.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch4_ex4_3: ch4_ex4_3.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch4_ex4_1: ch4_ex4_1.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch4_ex4_2: ch4_ex4_2.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch5_ex5_1: ch5_ex5_1.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch5_ex5_2: ch5_ex5_2.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch5_ex5_3: ch5_ex5_3.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch5_ex5_4: ch5_ex5_4.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch6_ex6_1: ch6_ex6_1.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch6_ex6_2: ch6_ex6_2.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch6_ex6_3: ch6_ex6_3.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch6_ex6_4: ch6_ex6_4.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch6_ex6_5: ch6_ex6_5.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch7_ex7_1: ch7_ex7_1.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch7_ex7_3_expanded: ch7_ex7_3_expanded.o
	$(LINK) -o $@ $^ -lcv -lhighgui
	
ch7_ex7_5_HistBackProj: ch7_ex7_5_HistBackProj.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch7_ex7_5: ch7_ex7_5.o
	$(LINK) -o $@ $^ -lcv -lhighgui
	
ch8_ex8_2: ch8_ex8_2.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch8_ex8_3: ch8_ex8_3.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch9_ex9_1: ch9_ex9_1.o
	$(LINK) -o $@ $^ -lcv -lhighgui
	
ch9_background: ch9_AvgBackground.o cv_yuv_codebook.o  ch9_backgroundAVG.o
	$(LINK) -o $@ $^ -lcv -lhighgui -lcxcore
	
ch9_watershed: ch9_watershed.o 
	$(LINK) -o $@ $^ -lcv -lhighgui

ch10_ex10_1: ch10_ex10_1.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch10_ex10_1b_Horn_Schunck: ch10_ex10_1b_Horn_Schunck.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch10_motempl: ch10_motempl.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch10_ex10_2: ch10_ex10_2.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch9_pg11: ch9_pg11.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch9_pg14: ch9_pg14.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch9_pg15: ch9_pg15.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch11_ex11_1: ch11_ex11_1.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch11_ex11_1_fromdisk: ch11_ex11_1_fromdisk.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch12_ex12_1: ch12_ex12_1.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch12_ex12_2: ch12_ex12_2.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch12_ex12_3: ch12_ex12_3.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch12_ex12_4: ch12_ex12_4.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch13_ex13_1: ch13_ex13_1.o
	$(LINK) -o $@ $^ -lcv -lhighgui

ch13_ex13_2: ch13_ex13_2.o
	$(LINK) -o $@ $^ -lcv -lhighgui -lml
	
ch13_ex13_3: ch13_ex13_3.o
	$(LINK) -o $@ $^ -lcv -lhighgui -lml

ch13_ex13_4: ch13_ex13_4.o
	$(LINK) -o $@ $^ -lcv -lhighgui


