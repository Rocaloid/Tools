#Replace with custom choice of samples and expected outputs.
SampleList = a      o      an     en     kan    \
             shang  lin    ling   reng   shen   \
             shai   sui    chui   song   chun   \
             ku     yue    hun    zhuang zuo    \
             ming   diu    luan   chuan  suan   \
             za		lun    chuo   kui    kun    \
             duan   bai    biao
OutputList = 54222  45962  17774  23752  20293  \
			 34536	24201  23752  26370  43405  \
			 37467  38773  33645  32891  27824  \
			 32622	35784  32476  31410  46198  \
			 23531	17077  26411  33542  31634  \
			 35937  26924  38704  32767	 28075  \
			 36992  39705  40850

RUDBList = $(SampleList:%=%.rudb)
AmplList = $(SampleList:%=%_ampl)
PosList = $(SampleList:%=%_position)

sample.dat : merge.m raw.dat
	@ octave merge.m 1> octave.log

merge.m :
	@ echo "function n = firstbg(list, x)" >  merge.m
	@ echo "    for i = 1:columns(list)"   >> merge.m
	@ echo "        if(list(i) >= x)"      >> merge.m
	@ echo "            n = i;"            >> merge.m
	@ echo "            break;"            >> merge.m
	@ echo "        end"                   >> merge.m
	@ echo "    end"                       >> merge.m
	@ echo "    n = i;"                    >> merge.m
	@ echo "end"                           >> merge.m
	@ echo "load raw.dat"                  >> merge.m
	@ echo "X = [" >> merge.m
	@ for i in $(AmplList); do echo -n $$i"(:) " >> merge.m; done
	@ echo "]';" >> merge.m
	@ echo "positions = [" >> merge.m
	@ for i in $(PosList);  do echo -n $$i";" >> merge.m; done
	@ echo "];" >> merge.m
	@ echo "out = [" $(OutputList) "];"                  >> merge.m
	@ echo "for i = 1:rows(positions)"                   >> merge.m
	@ echo "    y(i) = firstbg(positions(i, :), out(i))" >> merge.m
	@ echo "end"                                         >> merge.m
	@ echo "save sample.dat X y"                         >> merge.m

raw.dat : $(RUDBList)
	@ echo "" > raw.dat
	@ for i in $(RUDBList); \
	  do \
		  rudb2oct $$i -ap -t half -n 50 -h 8 >> raw.dat; \
	  done

.PHONY : clean
clean: 
	@ -rm -f *.dat
	@ -rm -f merge.m

