/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
 http://www.iem.at
 
 The IEM plug-in suite is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 The IEM plug-in suite is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this software.  If not, see <http://www.gnu.org/licenses/>.
 ==============================================================================
 */


#ifndef tDesign
#define tDesign
#define tDesignN 240
#define tDesignN4 60
const float tDesignX[240] = {
    8.9265353576272299e-01,
    4.1253405342503202e-01,
    -8.9265353580640705e-01,
    -1.8161861061384901e-01,
    -4.1253405347743499e-01,
    -1.8161861045138400e-01,
    -4.1253405331708998e-01,
    1.8161861040013599e-01,
    4.1253405332799598e-01,
    1.8161861058078901e-01,
    8.9265353586764395e-01,
    -8.9265353585506402e-01,
    -2.9209374259343301e-01,
    -2.9576702802688698e-01,
    2.9209374244786401e-01,
    9.0950707014761201e-01,
    2.9576702814539602e-01,
    9.0950707018885402e-01,
    2.9576702790731102e-01,
    -9.0950707010122100e-01,
    -2.9576702783533299e-01,
    -9.0950707021859101e-01,
    -2.9209374254089598e-01,
    2.9209374286193801e-01,
    -5.7522571803819200e-01,
    2.4120572786144000e-02,
    5.7522571811647805e-01,
    8.1763902255600296e-01,
    -2.4120573041502998e-02,
    8.1763902245837905e-01,
    -2.4120572818239001e-02,
    -8.1763902254357801e-01,
    2.4120572712950001e-02,
    -8.1763902260049504e-01,
    -5.7522571792546895e-01,
    5.7522571790822996e-01,
    -1.2883316172480000e-01,
    5.2247640694409000e-02,
    1.2883316184032501e-01,
    9.9028894794971700e-01,
    -5.2247640388510000e-02,
    9.9028894797773004e-01,
    -5.2247640390409002e-02,
    -9.9028894796062605e-01,
    5.2247640527807997e-02,
    -9.9028894797086797e-01,
    -1.2883316179047799e-01,
    1.2883316185741600e-01,
    7.1800638603474998e-01,
    6.5744687628673704e-01,
    -7.1800638610944201e-01,
    -2.2853978773721900e-01,
    -6.5744687624102105e-01,
    -2.2853978767899699e-01,
    -6.5744687636118504e-01,
    2.2853978770306499e-01,
    6.5744687630445398e-01,
    2.2853978778496700e-01,
    7.1800638588076005e-01,
    -7.1800638589101795e-01,
    8.6317647311780299e-01,
    4.6818181643848600e-01,
    -8.6317647319444601e-01,
    1.8902952912552701e-01,
    -4.6818181639267098e-01,
    1.8902952879217400e-01,
    -4.6818181641121298e-01,
    -1.8902952889785199e-01,
    4.6818181650886698e-01,
    -1.8902952900182299e-01,
    8.6317647313522905e-01,
    -8.6317647312333401e-01,
    7.7263285684713301e-01,
    -5.1705945056713198e-01,
    -7.7263285680608096e-01,
    3.6835851164800099e-01,
    5.1705945049400703e-01,
    3.6835851172049600e-01,
    5.1705945058344505e-01,
    -3.6835851156733002e-01,
    -5.1705945050236901e-01,
    -3.6835851146980297e-01,
    7.7263285693474903e-01,
    -7.7263285692748496e-01,
    -8.4781923191464803e-01,
    -6.6325775913630994e-02,
    8.4781923188301800e-01,
    -5.2612112834876201e-01,
    6.6325775846119997e-02,
    -5.2612112845923997e-01,
    6.6325775945784995e-02,
    5.2612112844953196e-01,
    -6.6325775877211002e-02,
    5.2612112850466897e-01,
    -8.4781923182172503e-01,
    8.4781923185026398e-01,
    9.8057432292300000e-03,
    9.4298381580892299e-01,
    -9.8057433796900004e-03,
    3.3269410922655401e-01,
    -9.4298381577404000e-01,
    3.3269410939799599e-01,
    -9.4298381577611401e-01,
    -3.3269410931902699e-01,
    9.4298381577508195e-01,
    -3.3269410945576500e-01,
    9.8057433011400005e-03,
    -9.8057432877129997e-03,
    7.8559924837115203e-01,
    -4.0515694493212501e-01,
    -7.8559924820178995e-01,
    -4.6763412061124199e-01,
    4.0515694513642297e-01,
    -4.6763412081180400e-01,
    4.0515694484198500e-01,
    4.6763412078672600e-01,
    -4.0515694499964300e-01,
    4.6763412089371298e-01,
    7.8559924831334105e-01,
    -7.8559924828109995e-01,
    -7.3733199913149206e-01,
    6.2085150094918595e-01,
    7.3733199906006097e-01,
    -2.6624225194863099e-01,
    -6.2085150107922105e-01,
    -2.6624225201162399e-01,
    -6.2085150107212395e-01,
    2.6624225211386399e-01,
    6.2085150118738697e-01,
    2.6624225193224998e-01,
    -7.3733199894794299e-01,
    7.3733199883500700e-01,
    7.2687146916565903e-01,
    -2.7488282182755001e-02,
    -7.2687146917293100e-01,
    -6.8622318644971203e-01,
    2.7488282351607000e-02,
    -6.8622318654562198e-01,
    2.7488282266836001e-02,
    6.8622318666118298e-01,
    -2.7488282251028999e-02,
    6.8622318660911197e-01,
    7.2687146907010702e-01,
    -7.2687146908018296e-01,
    6.6536338572051501e-01,
    5.8086026757708697e-01,
    -6.6536338567737996e-01,
    4.6892740834078300e-01,
    -5.8086026752845299e-01,
    4.6892740837261399e-01,
    -5.8086026764087695e-01,
    -4.6892740846833603e-01,
    5.8086026738675200e-01,
    -4.6892740837569902e-01,
    6.6536338565181896e-01,
    -6.6536338575173404e-01,
    -5.8012536730530395e-01,
    -7.7909959805351803e-01,
    5.8012536718680796e-01,
    2.3760971069593201e-01,
    7.7909959806473195e-01,
    2.3760971081928500e-01,
    7.7909959817022401e-01,
    -2.3760971081180199e-01,
    -7.7909959807496099e-01,
    -2.3760971060925301e-01,
    -5.8012536709009399e-01,
    5.8012536721841101e-01,
    9.5866802536020002e-01,
    1.0111360588989300e-01,
    -9.5866802532641004e-01,
    -2.6595423663417900e-01,
    -1.0111360600317100e-01,
    -2.6595423671545498e-01,
    -1.0111360582543800e-01,
    2.6595423628673898e-01,
    1.0111360580244400e-01,
    2.6595423651585398e-01,
    9.5866802544950003e-01,
    -9.5866802547859997e-01,
    -7.8443181441708498e-01,
    2.8431902482284799e-01,
    7.8443181444342203e-01,
    5.5120723943467698e-01,
    -2.8431902464016101e-01,
    5.5120723940835703e-01,
    -2.8431902471494003e-01,
    -5.5120723941764904e-01,
    2.8431902447710600e-01,
    -5.5120723922716397e-01,
    -7.8443181465489997e-01,
    7.8443181454213895e-01,
    1.6666387853511799e-01,
    9.7946877889236195e-01,
    -1.6666387832233501e-01,
    1.1341985185260300e-01,
    -9.7946877890805095e-01,
    1.1341985202353200e-01,
    -9.7946877889141803e-01,
    -1.1341985194229900e-01,
    9.7946877888779205e-01,
    -1.1341985188733300e-01,
    1.6666387851331199e-01,
    -1.6666387852599199e-01,
    9.0354263539086999e-01,
    9.9002690511180005e-02,
    -9.0354263538353297e-01,
    4.1690427395825003e-01,
    -9.9002690414932995e-02,
    4.1690427384396400e-01,
    -9.9002690464191995e-02,
    -4.1690427420603599e-01,
    9.9002690128043996e-02,
    -4.1690427411374398e-01,
    9.0354263527927503e-01,
    -9.0354263523439904e-01,
    2.7876240453609202e-01,
    3.4931218558605598e-01,
    -2.7876240454052498e-01,
    -8.9457952073414404e-01,
    -3.4931218546670101e-01,
    -8.9457952078886405e-01,
    -3.4931218555104099e-01,
    8.9457952078521896e-01,
    3.4931218554962301e-01,
    8.9457952078180503e-01,
    2.7876240443794997e-01,
    -2.7876240444325900e-01,
    5.5589623017941503e-01,
    -6.7683321168156696e-01,
    -5.5589623031489199e-01,
    4.8257246591028302e-01,
    6.7683321145769204e-01,
    4.8257246590298097e-01,
    6.7683321163559196e-01,
    -4.8257246615058602e-01,
    -6.7683321143828601e-01,
    -4.8257246597237302e-01,
    5.5589623019269097e-01,
    -5.5589623019433798e-01
};


const float tDesignY[240]= {
    4.1253405365736101e-01,
    -1.8161861064178200e-01,
    -4.1253405362785300e-01,
    8.9265353574047501e-01,
    -1.8161861042265401e-01,
    -8.9265353576281203e-01,
    1.8161861061182699e-01,
    -8.9265353581230000e-01,
    1.8161861042040001e-01,
    8.9265353581090401e-01,
    -4.1253405347255800e-01,
    4.1253405353515998e-01,
    -2.9576702799317001e-01,
    9.0950707008926002e-01,
    2.9576702803971300e-01,
    -2.9209374292672102e-01,
    9.0950707008444098e-01,
    2.9209374268920701e-01,
    -9.0950707014841903e-01,
    2.9209374315927200e-01,
    -9.0950707004729303e-01,
    -2.9209374272177602e-01,
    2.9576702779314701e-01,
    -2.9576702774761399e-01,
    2.4120572825077999e-02,
    8.1763902251123799e-01,
    -2.4120572979212999e-02,
    -5.7522571810348000e-01,
    8.1763902244075704e-01,
    5.7522571822911805e-01,
    -8.1763902258125998e-01,
    5.7522571812388201e-01,
    -8.1763902252729603e-01,
    -5.7522571803517397e-01,
    -2.4120572711051998e-02,
    2.4120572594154999e-02,
    5.2247640720239998e-02,
    9.9028894795889499e-01,
    -5.2247640320038001e-02,
    -1.2883316192479599e-01,
    9.9028894796758105e-01,
    1.2883316187800101e-01,
    -9.9028894796219002e-01,
    1.2883316189664901e-01,
    -9.9028894795325095e-01,
    -1.2883316193620500e-01,
    -5.2247640337643002e-02,
    5.2247640551545002e-02,
    6.5744687625599296e-01,
    -2.2853978783192200e-01,
    -6.5744687617143405e-01,
    7.1800638594742205e-01,
    -2.2853978771380001e-01,
    -7.1800638603135902e-01,
    2.2853978786054899e-01,
    -7.1800638585738497e-01,
    2.2853978787401699e-01,
    7.1800638581385301e-01,
    -6.5744687636348498e-01,
    6.5744687637155796e-01,
    4.6818181665313802e-01,
    1.8902952919749200e-01,
    -4.6818181657642000e-01,
    8.6317647306438905e-01,
    1.8902952889744301e-01,
    -8.6317647314368795e-01,
    -1.8902952912813800e-01,
    -8.6317647308972001e-01,
    -1.8902952893055500e-01,
    8.6317647310665901e-01,
    -4.6818181664864200e-01,
    4.6818181669876202e-01,
    -5.1705945069559001e-01,
    3.6835851158551502e-01,
    5.1705945064739101e-01,
    7.7263285680605398e-01,
    3.6835851181658802e-01,
    -7.7263285680247595e-01,
    -3.6835851148711701e-01,
    -7.7263285685946703e-01,
    -3.6835851166595601e-01,
    7.7263285685565097e-01,
    5.1705945069191905e-01,
    -5.1705945063377801e-01,
    -6.6325775900167000e-02,
    -5.2612112825768598e-01,
    6.6325775819851995e-02,
    -8.4781923176695695e-01,
    -5.2612112840709802e-01,
    8.4781923170908002e-01,
    5.2612112834437996e-01,
    8.4781923170069196e-01,
    5.2612112830638802e-01,
    -8.4781923166521300e-01,
    6.6325775941005000e-02,
    -6.6325775996654998e-02,
    9.4298381584259305e-01,
    3.3269410953974798e-01,
    -9.4298381578729096e-01,
    9.8057432042720008e-03,
    3.3269410963564700e-01,
    -9.8057432989099996e-03,
    -3.3269410963009799e-01,
    -9.8057431885070004e-03,
    -3.3269410963519902e-01,
    9.8057433897619992e-03,
    -9.4298381575252399e-01,
    9.4298381579137902e-01,
    -4.0515694531226898e-01,
    -4.6763412064985899e-01,
    4.0515694543405101e-01,
    7.8559924833462302e-01,
    -4.6763412086820100e-01,
    -7.8559924814560900e-01,
    4.6763412086133199e-01,
    -7.8559924824985705e-01,
    4.6763412087109801e-01,
    7.8559924823423999e-01,
    4.0515694511710398e-01,
    -4.0515694519737000e-01,
    6.2085150101376396e-01,
    -2.6624225215489500e-01,
    -6.2085150108873699e-01,
    -7.3733199910325498e-01,
    -2.6624225223379999e-01,
    7.3733199899622204e-01,
    2.6624225222255998e-01,
    7.3733199883297396e-01,
    2.6624225232837401e-01,
    -7.3733199893899004e-01,
    -6.2085150118329702e-01,
    6.2085150130578604e-01,
    -2.7488282350427998e-02,
    -6.8622318644832503e-01,
    2.7488282371885001e-02,
    7.2687146918540602e-01,
    -6.8622318649112002e-01,
    -7.2687146908979405e-01,
    6.8622318647033498e-01,
    -7.2687146898342203e-01,
    6.8622318652309200e-01,
    7.2687146903349797e-01,
    2.7488282335550000e-02,
    -2.7488282309715999e-02,
    5.8086026773927102e-01,
    4.6892740848863801e-01,
    -5.8086026771957500e-01,
    6.6536338582186305e-01,
    4.6892740867883198e-01,
    -6.6536338569880304e-01,
    -4.6892740855276199e-01,
    -6.6536338584794696e-01,
    -4.6892740865451898e-01,
    6.6536338565135600e-01,
    -5.8086026779121203e-01,
    5.8086026754801701e-01,
    -7.7909959792443395e-01,
    2.3760971090993399e-01,
    7.7909959797773198e-01,
    -5.8012536727610997e-01,
    2.3760971114731999e-01,
    5.8012536704742601e-01,
    -2.3760971084964200e-01,
    5.8012536715725604e-01,
    -2.3760971104512801e-01,
    -5.8012536702235895e-01,
    7.7909959815196606e-01,
    -7.7909959796671602e-01,
    1.0111360590053900e-01,
    -2.6595423647719901e-01,
    -1.0111360609543200e-01,
    9.5866802529455497e-01,
    -2.6595423665631701e-01,
    -9.5866802524616201e-01,
    2.6595423641466398e-01,
    -9.5866802539358298e-01,
    2.6595423626066400e-01,
    9.5866802532257700e-01,
    -1.0111360590910100e-01,
    1.0111360578949700e-01,
    2.8431902500722900e-01,
    5.5120723932070903e-01,
    -2.8431902488813099e-01,
    -7.8443181429188802e-01,
    5.5120723934750404e-01,
    7.8443181440099796e-01,
    -5.5120723916013703e-01,
    7.8443181442674303e-01,
    -5.5120723939406702e-01,
    -7.8443181451083199e-01,
    -2.8431902475772902e-01,
    2.8431902468988401e-01,
    9.7946877886664996e-01,
    1.1341985201124800e-01,
    -9.7946877887722195e-01,
    1.6666387846509201e-01,
    1.1341985223322899e-01,
    -1.6666387821316500e-01,
    -1.1341985208875500e-01,
    -1.6666387838378499e-01,
    -1.1341985225265100e-01,
    1.6666387842006100e-01,
    -9.7946877885883998e-01,
    9.7946877885240302e-01,
    9.9002690679599006e-02,
    4.1690427375369199e-01,
    -9.9002690647922997e-02,
    9.0354263519376798e-01,
    4.1690427369973199e-01,
    -9.0354263523751699e-01,
    -4.1690427393725399e-01,
    -9.0354263511014699e-01,
    -4.1690427406438002e-01,
    9.0354263513138600e-01,
    -9.9002690467102000e-02,
    9.9002690245829000e-02,
    3.4931218553706300e-01,
    -8.9457952060851498e-01,
    -3.4931218550347298e-01,
    2.7876240472791702e-01,
    -8.9457952067772295e-01,
    -2.7876240465867702e-01,
    8.9457952068279800e-01,
    -2.7876240468046898e-01,
    8.9457952067922997e-01,
    2.7876240455590801e-01,
    -3.4931218550650001e-01,
    3.4931218542878700e-01,
    -6.7683321173667099e-01,
    4.8257246604011600e-01,
    6.7683321152298703e-01,
    5.5589623016467204e-01,
    4.8257246609289500e-01,
    -5.5589623036790903e-01,
    -4.8257246607198101e-01,
    -5.5589623023008405e-01,
    -4.8257246632773698e-01,
    5.5589623026777002e-01,
    6.7683321158945298e-01,
    -6.7683321145553699e-01
  };


const float tDesignZ[240] = {
   -1.8161861045425301e-01,
    8.9265353583193796e-01,
    -1.8161861030657500e-01,
    4.1253405363523998e-01,
    -8.9265353585230400e-01,
    -4.1253405365843199e-01,
    8.9265353588791796e-01,
    4.1253405357391099e-01,
    -8.9265353592182495e-01,
    -4.1253405349739902e-01,
    1.8161861035833901e-01,
    1.8161861027797099e-01,
    9.0950707017034704e-01,
    -2.9209374281177602e-01,
    9.0950707020196198e-01,
    -2.9576702773393398e-01,
    2.9209374270678301e-01,
    2.9576702784167502e-01,
    -2.9209374274865102e-01,
    -2.9576702764692703e-01,
    2.9209374313641401e-01,
    2.9576702771806901e-01,
    -9.0950707025226596e-01,
    -9.0950707016396903e-01,
    8.1763902259740295e-01,
    -5.7522571816230095e-01,
    8.1763902253778098e-01,
    2.4120572671468998e-02,
    5.7522571825177704e-01,
    -2.4120572984525999e-02,
    -5.7522571806142397e-01,
    2.4120572606111002e-02,
    5.7522571814254597e-01,
    -2.4120572792227998e-02,
    -8.1763902268007005e-01,
    -8.1763902269564603e-01,
    9.9028894797385303e-01,
    -1.2883316185025101e-01,
    9.9028894797993805e-01,
    5.2247640684557999e-02,
    1.2883316190753799e-01,
    -5.2247640268992000e-02,
    -1.2883316194820901e-01,
    5.2247640547187002e-02,
    1.2883316196120001e-01,
    -5.2247640255526001e-02,
    -9.9028894798549405e-01,
    -9.9028894796550004e-01,
    -2.2853978759628599e-01,
    7.1800638593159605e-01,
    -2.2853978760487700e-01,
    6.5744687630237397e-01,
    -7.1800638601105404e-01,
    -6.5744687623094500e-01,
    7.1800638585431498e-01,
    6.5744687641257704e-01,
    -7.1800638590197496e-01,
    -6.5744687643164801e-01,
    2.2853978777085099e-01,
    2.2853978771540101e-01,
    1.8902952894000100e-01,
    8.6317647317783996e-01,
    1.8902952878003301e-01,
    4.6818181667670800e-01,
    -8.6317647326839797e-01,
    -4.6818181666510000e-01,
    8.6317647320782098e-01,
    4.6818181672193099e-01,
    -8.6317647319812296e-01,
    -4.6818181664872199e-01,
    -1.8902952887156099e-01,
    -1.8902952880174401e-01,
    3.6835851146215198e-01,
    7.7263285687428596e-01,
    3.6835851161591499e-01,
    -5.1705945062457304e-01,
    -7.7263285681305605e-01,
    5.1705945057827296e-01,
    7.7263285691028005e-01,
    -5.1705945060222902e-01,
    -7.7263285687927497e-01,
    5.1705945067741199e-01,
    -3.6835851128353098e-01,
    -3.6835851138037801e-01,
    -5.2612112811300205e-01,
    -8.4781923182380903e-01,
    -5.2612112817409695e-01,
    -6.6325775917910001e-02,
    8.4781923173637197e-01,
    6.6325775781366003e-02,
    -8.4781923176749596e-01,
    -6.6325775965612996e-02,
    8.4781923179643603e-01,
    6.6325775981759996e-02,
    5.2612112825759405e-01,
    5.2612112820459001e-01,
    3.3269410944389199e-01,
    9.8057432149499992e-03,
    3.3269410959620699e-01,
    9.4298381591953195e-01,
    -9.8057433158039997e-03,
    -9.4298381585806201e-01,
    9.8057433046670004e-03,
    9.4298381588707003e-01,
    -9.8057432307629994e-03,
    -9.4298381583673496e-01,
    -3.3269410969706498e-01,
    -3.3269410958733098e-01,
    -4.6763412046589597e-01,
    7.8559924845769802e-01,
    -4.6763412064490401e-01,
    -4.0515694521533901e-01,
    -7.8559924822236604e-01,
    4.0515694535034702e-01,
    7.8559924837830497e-01,
    -4.0515694517715600e-01,
    -7.8559924829118200e-01,
    4.0515694508395300e-01,
    4.6763412073210597e-01,
    4.6763412071672700e-01,
    -2.6624225199189000e-01,
    -7.3733199912700997e-01,
    -2.6624225201488300e-01,
    6.2085150106584996e-01,
    7.3733199898902502e-01,
    -6.2085150116595100e-01,
    -7.3733199899905999e-01,
    6.2085150131598299e-01,
    7.3733199886379697e-01,
    -6.2085150126795896e-01,
    2.6624225210487901e-01,
    2.6624225213201003e-01,
    -6.8622318646806102e-01,
    7.2687146919063295e-01,
    -6.8622318645949898e-01,
    -2.7488282286340999e-02,
    -7.2687146914384504e-01,
    2.7488282420281000e-02,
    7.2687146916667400e-01,
    -2.7488282348185001e-02,
    -7.2687146911746503e-01,
    2.7488282323948000e-02,
    6.8622318656986903e-01,
    6.8622318656023196e-01,
    4.6892740835271601e-01,
    6.6536338576630805e-01,
    4.6892740843831798e-01,
    5.8086026763281295e-01,
    -6.6536338567472297e-01,
    -5.8086026774807797e-01,
    6.6536338566542697e-01,
    5.8086026749996100e-01,
    -6.6536338581556298e-01,
    -5.8086026779993805e-01,
    -4.6892740838585001e-01,
    -4.6892740854532600e-01,
    2.3760971091870700e-01,
    -5.8012536713553897e-01,
    2.3760971103325801e-01,
    -7.7909959801411399e-01,
    5.8012536702325002e-01,
    7.7909959814677399e-01,
    -5.8012536700349904e-01,
    -7.7909959806727602e-01,
    5.8012536705136897e-01,
    7.7909959822949504e-01,
    -2.3760971069808601e-01,
    -2.3760971099221501e-01,
    -2.6595423638995602e-01,
    9.5866802533712003e-01,
    -2.6595423643765997e-01,
    1.0111360588055800e-01,
    -9.5866802527548201e-01,
    -1.0111360612560200e-01,
    9.5866802536126705e-01,
    1.0111360585552200e-01,
    -9.5866802540641505e-01,
    -1.0111360592610601e-01,
    2.6595423606480800e-01,
    2.6595423600538598e-01,
    5.5120723920251602e-01,
    -7.8443181440086196e-01,
    5.5120723922646697e-01,
    2.8431902490255601e-01,
    7.8443181444824905e-01,
    -2.8431902465254599e-01,
    -7.8443181455280397e-01,
    2.8431902456350300e-01,
    7.8443181447462895e-01,
    -2.8431902470079701e-01,
    -5.5120723899277202e-01,
    -5.5120723918823999e-01,
    1.1341985195328499e-01,
    1.6666387834456400e-01,
    1.1341985217465900e-01,
    9.7946877889022399e-01,
    -1.6666387810129701e-01,
    -9.7946877891329798e-01,
    1.6666387829736801e-01,
    9.7946877889367301e-01,
    -1.6666387820714201e-01,
    -9.7946877889386497e-01,
    -1.1341985205277499e-01,
    -1.1341985208972701e-01,
    4.1690427350786502e-01,
    9.0354263529589696e-01,
    4.1690427353128801e-01,
    9.9002690581850003e-02,
    -9.0354263533134105e-01,
    -9.9002690663844997e-02,
    9.0354263521634803e-01,
    9.9002690301575005e-02,
    -9.0354263519452305e-01,
    -9.9002690496392001e-02,
    -4.1690427380018302e-01,
    -4.1690427394998802e-01,
    -8.9457952069817503e-01,
    2.7876240476243103e-01,
    -8.9457952070990998e-01,
    3.4931218529186597e-01,
    -2.7876240468989599e-01,
    -3.4931218520698398e-01,
    2.7876240456792301e-01,
    3.4931218519892898e-01,
    -2.7876240458114898e-01,
    -3.4931218530707497e-01,
    8.9457952074069202e-01,
    8.9457952076938196e-01,
    4.8257246581475999e-01,
    5.5589623005087596e-01,
    4.8257246595840098e-01,
    -6.7683321168067301e-01,
    -5.5589623027763901e-01,
    6.7683321151895703e-01,
    5.5589623007919098e-01,
    -6.7683321145561604e-01,
    -5.5589623009739997e-01,
    6.7683321155172704e-01,
    -4.8257246600594900e-01,
    -4.8257246619187499e-01
};


#endif
