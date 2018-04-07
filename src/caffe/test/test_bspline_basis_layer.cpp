#include <vector>

#include "gtest/gtest.h"

#include "caffe/blob.hpp"
#include "caffe/common.hpp"
#include "caffe/filler.hpp"
#include "caffe/layers/bspline_basis_layer.hpp"

#include "caffe/test/test_caffe_main.hpp"
#include "caffe/test/test_gradient_check_util.hpp"
#include "caffe/util/benchmark.hpp"

namespace caffe {

#ifndef CPU_ONLY
extern cudaDeviceProp CAFFE_TEST_CUDA_PROP;
#endif

template <typename TypeParam>
class BSplineBasisLayerTest : public MultiDeviceTest<TypeParam> {
  typedef typename TypeParam::Dtype Dtype;
 protected:
  BSplineBasisLayerTest()
      :
    blob_X1(new Blob<Dtype>()),
    blob_X2(new Blob<Dtype>()),
    blob_X2b(new Blob<Dtype>()),
    blob_X3(new Blob<Dtype>()),

    blob_Y1(new Blob<Dtype>()),
    blob_Y2(new Blob<Dtype>()),
    blob_Y2b(new Blob<Dtype>()),
    blob_Y3(new Blob<Dtype>()),

    blob_top_(new Blob<Dtype>())
  {
    // X1/X2/X3
    vector<int> shape(2);
    shape[0]=10; shape[1]=1;
    blob_X1->Reshape(shape);

    shape[0]=10; shape[1]=2;
    blob_X2->Reshape(shape);

    shape[0]=10; shape[1]=3;
    blob_X3->Reshape(shape);

    // X2b
    vector<int> shape2(3);
    shape2[0]=2; shape2[1]=10; shape2[2]=2;
    blob_X2b->Reshape(shape2);

    Dtype* X1 = blob_X1->mutable_cpu_data();
    Dtype* X2 = blob_X2->mutable_cpu_data();
    Dtype* X2b = blob_X2b->mutable_cpu_data();
    Dtype* X3 = blob_X3->mutable_cpu_data();
    const Dtype x1[10] = {
       0.682669110596180,
       0.775540385395288,
       0.128264394588768,
       0.487508248072118,
       0.265271934447810,
       0.056655199732631,
       0.725826490670442,
       0.061400201870129,
       0.640846163267270,
       0.707459864905104
    };
    const Dtype x2[10][2] = {
        {0.878208859125152, 0.556152194039896},
        {0.658815313829109, 0.820074574556202},
        {0.186809764010832, 0.675696801394224},
        {0.725164783187210, 0.909776101121679},
        {0.832602044101804, 0.303071619477123},
        {0.900915545178577, 0.947297151898965},
        {0.939058439107612, 0.457924959249794},
        {0.213631788035855, 0.303759654052556},
        {0.988321385579184, 0.274314521113411},
        {0.204750217031687, 0.813572845654562}
    };
    const Dtype x3[10][3] = {
        {0.402940567815676, 0.215625262353569, 0.121408637613058},
        {0.886591678950936, 0.846945045283064, 0.807418946875259},
        {0.405726168071851, 0.471708307508379, 0.110006193397567},
        {0.752652072580531, 0.502196621615440, 0.595858316868544},
        {0.698478043079376, 0.456705866148695, 0.952968080295250},
        {0.115230368683115, 0.626595487585291, 0.644060203805566},
        {0.471115160267800, 0.254925100132823, 0.979889175388962},
        {0.280331819318235, 0.901308518135920, 0.980417774757370},
        {0.799139697104692, 0.490077926311642, 0.379838967695832},
        {0.189668979961425, 0.389833993278444, 0.942920924862847}
    };
    caffe_copy(blob_X1->count(), x1, X1);
    caffe_copy(blob_X2->count(), &x2[0][0], X2);
    caffe_copy(blob_X2->count(), &x2[0][0], X2b); //simply tile X2
    caffe_copy(blob_X2->count(), &x2[0][0], X2b+blob_X2->count()); //simply tile X2
    caffe_copy(blob_X3->count(), &x3[0][0], X3);

    //Y
    shape[0]=10; shape[1]=7;
    blob_Y1->Reshape(shape);
    shape[0]=10; shape[1]=49;
    blob_Y2->Reshape(shape);
    vector<int> shapeb(3);
    shapeb[0]=2; shapeb[1]=10; shapeb[2]=49;
    blob_Y2b->Reshape(shapeb);
    shape[0]=10; shape[1]=343;
    blob_Y3->Reshape(shape);

    Dtype* Y1 = blob_Y1->mutable_cpu_data();
    Dtype* Y2 = blob_Y2->mutable_cpu_data();
    Dtype* Y2b = blob_Y2b->mutable_cpu_data();
    Dtype* Y3 = blob_Y3->mutable_cpu_data();
    const Dtype y1[10][7] = {
        {0, 0, 0.00325590539602035, 0.327828318629522, 0.571390918265036, 0.0975248577094226, 0},
        {0, 0, 0, 0.120626676494321, 0.545724125405129, 0.332582940075224, 0.00106625802532533},
        {0.115460340440731, 0.590985911960485, 0.271045220829324, 0.0225085267694610, 0, 0, 0},
        {0, 3.11881802685569e-05, 0.192825749031627, 0.664232341167560, 0.142910721620545, 0, 0},
        {0, 0.206925989981319, 0.594073605877622, 0.198962410503333, 3.79936377263954e-05, 0, 0},
        {0.462570001419065, 0.469123439803885, 0.0663667985582018, 0.00193976021884779, 0, 0, 0},
        {0, 0, 0.000150677300694979, 0.219236521850146, 0.596347042149199, 0.184265758699959, 0},
        {0.429342266527096, 0.491289034222737, 0.0768996024274005, 0.00246909682276731, 0, 0, 0},
        {0, 0, 0.0138722127466929, 0.438674182479853, 0.502748713209978, 0.0447048915634762, 0},
        {0, 0, 0.000821155333231079, 0.263760775607902, 0.592554250216698, 0.142863818842169, 0}
    };
    const Dtype y2[10][49] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.00149722549243634, 0.0119835269890119, 0.00573439811008304, 5.45878504305957e-05, 0, 0, 0, 0.0194253780381603, 0.155477276581271, 0.0743995154186245, 0.000708236425482824, 0, 0, 0, 0.0462960558376861, 0.370545410439773, 0.177314650626157, 0.00168792355217100, 0, 0, 0, 0.0104796185271749, 0.0838770058948148, 0.0401371102617517, 0.000382079954970741, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.000502459179977546, 0.00351981746824596, 0.00388675708620213, 0.000178096682586773, 0, 0, 0, 0.0243112400477250, 0.170304635291025, 0.188058828050627, 0.00861712030470618, 0, 0, 0, 0.0333349937594824, 0.233517662755688, 0.257861789327766, 0.0118155902791544, 0, 0, 0, 0.00398202014198052, 0.0278947715817962, 0.0308027907957300, 0.00141142724730820, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 7.06612243912595e-05, 0.00559241536727387, 0.00908400521780483, 0.00140133333721062, 0, 0, 0, 0.00200945699700256, 0.159036561943628, 0.258330053052175, 0.0398509805604097, 0, 0, 0, 0.00199133549391402, 0.157602352825069, 0.256000404365375, 0.0394916000569284, 0, 0, 0, 0.000304283702229655, 0.0240822440740299, 0.0391178438041482, 0.00603446797840979, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1.28004750065300e-06, 2.48815123815010e-05, 9.45782713331810e-05, 4.26528467787916e-05, 0, 0, 0, 0.00172963966483937, 0.0336206669785940, 0.127797077410298, 0.0576338421576381, 0, 0, 0, 0.00467234095014328, 0.0908207774651164, 0.345223071736286, 0.155688474484791, 0, 0, 0, 0.00143091842294066, 0.0278141353654089, 0.105725600644890, 0.0476800620410594, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.00611398101766208, 0.0293035362364001, 0.0145383098572826, 7.97801630607648e-05, 0, 0, 0, 0.0485512720867327, 0.232700094554910, 0.115449072465728, 0.000633536216860589, 0, 0, 0, 0.0631197981240130, 0.302525193686086, 0.150091271236384, 0.000823638112736880, 0, 0, 0, 0.00440755021503543, 0.0211248296432284, 0.0104806231083492, 5.75132755300075e-05, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1.62022853294122e-05, 0.000602598472969868, 0.00465735957597754, 0.00510020600283131, 0, 0, 0, 0.000278807323439115, 0.0103694549220312, 0.0801433829408394, 0.0877638404538016, 0, 0, 0, 0.000922961769919923, 0.0343269694278053, 0.265306081827511, 0.290532789888855, 0, 0, 0, 0.000343489040405406, 0.0127751096232356, 0.0987361930154014, 0.108124553429647},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2.87715369952250e-06, 0.000632993876583599, 0.00154682540365746, 0.000231482435508113, 0, 0, 0, 9.04019430694393e-05, 0.0198890578573231, 0.0486022078358248, 0.00727332083783101, 0, 0, 0, 0.000583073483986164, 0.128280232308327, 0.313473999452497, 0.0469113868250133, 0, 0, 0, 0.000515420628733499, 0.113396132402423, 0.277102235527562, 0.0414683520279623, 0, 0},
        {0, 0.000372246572640899, 0.00180123179112489, 0.000899966839049226, 5.10204097711308e-06, 0, 0, 0, 0.0446893308508034, 0.216243343441656, 0.108043750516465, 0.000612515504502757, 0, 0, 0, 0.0632796386490845, 0.306198378287239, 0.152988853509559, 0.000867315734068535, 0, 0, 0, 0.0125750938497997, 0.0608485355766837, 0.0304023416050874, 0.000172355231258539, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3.12488123408347e-06, 1.01323834988161e-05, 3.73049416507935e-06, 2.60512501120131e-09, 0, 0, 0, 0.000584852275606677, 0.00189637528683238, 0.000698198695613468, 4.87574783458237e-07, 0, 0, 0, 0.0240019742701093, 0.0778260643575451, 0.0286536409731070, 2.00097663896054e-05, 0, 0, 0, 0.159330842139044, 0.516628017133469, 0.190209717551463, 0.000132829612013698, 0, 0},
        {0, 0, 0, 0.000409812980907951, 0.00269208493612600, 0.00273025331843674, 9.75044428371802e-05, 0, 0, 0, 0.0276410348067602, 0.181575540280232, 0.184149918431044, 0.00657647225401485, 0, 0, 0, 0.0347337160521037, 0.228167769484464, 0.231402732297043, 0.00826399306293788, 0, 0, 0, 0.00632787761397843, 0.0415681903596573, 0.0421575441947914, 0.00150555548466489, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

    };
    const Dtype y3[10][343] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5.17543357467610e-06, 2.26680980584055e-05, 9.46095672264690e-06, 7.25953582757041e-07, 0, 0, 0, 0.000722206450320202, 0.00316322224950899, 0.00132023025176659, 0.000101303272959697, 0, 0, 0, 0.00105061390013784, 0.00460162777982109, 0.00192057583156932, 0.000147368701364724, 0, 0, 0, 0.000212900280813246, 0.000932490847868482, 0.000389192579510510, 2.98633379013137e-05, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.000142218043408035, 0.000622906758850568, 0.000259981841994203, 1.99488017100520e-05, 0, 0, 0, 0.0198458325895197, 0.0869235925258853, 0.0362791948819654, 0.00278375773996093, 0, 0, 0, 0.0288702871168119, 0.126450178501068, 0.0527763583555688, 0.00404961015637917, 0, 0, 0, 0.00585038160405255, 0.0256243311726892, 0.0106947961689132, 0.000820627957962659, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.000192871154939482, 0.000844764441417784, 0.000352578315149869, 2.70538697712043e-05, 0, 0, 0, 0.0269142266378559, 0.117882747366002, 0.0492005799649744, 0.00377523524801673, 0, 0, 0, 0.0391528774142830, 0.171487325977562, 0.0715734582308524, 0.00549194018704507, 0, 0, 0, 0.00793408367722325, 0.0347508250668925, 0.0145039099084111, 0.00111290704213798, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1.34992716142621e-05, 5.91260245646750e-05, 2.46773574980686e-05, 1.89353113156269e-06, 0, 0, 0, 0.00188375734975106, 0.00825074763423497, 0.00344360458013958, 0.000264233011082994, 0, 0, 0, 0.00274035444471287, 0.0120025930912322, 0.00500950778945398, 0.000384387143310596, 0, 0, 0, 0.000555315545254688, 0.00243224979155065, 0.00101514515938821, 7.78936303251753e-05, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4.53318857231958e-05, 0.000280304834725128, 0.000262178068527140, 7.20908310772514e-06, 0, 0, 0, 0.000417080646320581, 0.00257897327165718, 0.00241219610717469, 6.63279057109741e-05, 0, 0, 0, 0.000653782247538725, 0.00404259213838073, 0.00378116560038287, 0.000103970317617966, 0, 0, 0, 6.91177687033281e-05, 0.000427382281232623, 0.000399744303826120, 1.09917275854213e-05, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.000650050811272391, 0.00401951920397185, 0.00375958474757844, 0.000103376911150840, 0, 0, 0, 0.00598085890717703, 0.0369819971250995, 0.0345904436005992, 0.000951129833439847, 0, 0, 0, 0.00937511585119373, 0.0579700194967430, 0.0542212115571984, 0.00149091502013612, 0, 0, 0, 0.000991135949942310, 0.00612858243610534, 0.00573225897970360, 0.000157619329533696, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.00174306874432410, 0.0107780779135402, 0.0100810806655475, 0.000277198427549214, 0, 0, 0, 0.0160372820778532, 0.0991648070791086, 0.0927520126841981, 0.00255039245504223, 0, 0, 0, 0.0251387601265305, 0.155442816685125, 0.145390633325637, 0.00399779113721258, 0, 0, 0, 0.00265766624048792, 0.0164333930611965, 0.0153706776280073, 0.000422645925591161, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.000475218131748174, 0.00293846015344330, 0.00274843567442993, 7.55734501535964e-05, 0, 0, 0, 0.00437229297592111, 0.0270356028749603, 0.0252872632403028, 0.000695321249753562, 0, 0, 0, 0.00685365661032934, 0.0423788477530317, 0.0396382904390180, 0.00108992995344097, 0, 0, 0, 0.000724567628056802, 0.00448028591772691, 0.00419055457787894, 0.000115227243793125},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8.52897172618205e-07, 2.90401905233477e-06, 1.03135965597241e-06, 6.89716104587678e-08, 0, 0, 0, 0.000538605877480229, 0.00183389250207099, 0.000651305210448145, 4.35556781813820e-05, 0, 0, 0, 0.00154086888798446, 0.00524648545160537, 0.00186328441133388, 0.000124606121490419, 0, 0, 0, 0.000273636816094061, 0.000931702616527606, 0.000330893314655751, 2.21283086551695e-05, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2.51326487176251e-05, 8.55738453060766e-05, 3.03914712901606e-05, 2.03241294824300e-06, 0, 0, 0, 0.0158713063550258, 0.0540400150453647, 0.0191922609051486, 0.00128347190556420, 0, 0, 0, 0.0454053756125730, 0.154600202803526, 0.0549061177296741, 0.00367181646278748, 0, 0, 0, 0.00806336120682623, 0.0274548390148816, 0.00975056045117915, 0.000652063374109124, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3.50730407919007e-05, 0.000119419763545855, 4.24118175629522e-05, 2.83626700236035e-06, 0, 0, 0, 0.0221486792524259, 0.0754137645170614, 0.0267831280808466, 0.00179110697821289, 0, 0, 0, 0.0633639776262241, 0.215747225065125, 0.0766224256143479, 0.00512408262362754, 0, 0, 0, 0.0112525583635965, 0.0383136970369831, 0.0136070737426205, 0.000909965582059621, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2.56281186910514e-06, 8.72608649010330e-06, 3.09906147247307e-06, 2.07248033631544e-07, 0, 0, 0, 0.00161841964059837, 0.00551053705166210, 0.00195706209064164, 0.000130877452281271, 0, 0, 0, 0.00463005061060174, 0.0157648021568488, 0.00559885477193706, 0.000374420337376068, 0, 0, 0, 0.000822232389347975, 0.00279960891039574, 0.000994278491513903, 6.64918279545953e-05, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.00102351204372072, 0.0143532855369143, 0.0104541185003363, 0.000369243894727668, 0, 0, 0, 0.00420340494774478, 0.0589467137318042, 0.0429334404985411, 0.00151642730883813, 0, 0, 0, 0.00107891639047263, 0.0151302518792208, 0.0110200166838815, 0.000389231658335362, 0, 0, 0, 1.06937076314269e-09, 1.49963881738193e-08, 1.09225179588988e-08, 3.85787962059828e-10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.00368105931997569, 0.0516215669586663, 0.0375982193603708, 0.00132798503776305, 0, 0, 0, 0.0151175387270282, 0.212001755422043, 0.154410045544026, 0.00545382823046427, 0, 0, 0, 0.00388032095859489, 0.0544159250839091, 0.0396334712125363, 0.00139987099549551, 0, 0, 0, 3.84599012617972e-09, 5.39344844956933e-08, 3.92828172143883e-08, 1.38748574771266e-09, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.00163615320213331, 0.0229446973647571, 0.0167116152318315, 0.000590261330517079, 0, 0, 0, 0.00671942700362810, 0.0942303073219558, 0.0686320073918496, 0.00242411157971181, 0, 0, 0, 0.00172472079633640, 0.0241867305940813, 0.0176162417389345, 0.000622213122028322, 0, 0, 0, 1.70946146566407e-09, 2.39727404103938e-08, 1.74603834351040e-08, 6.16708140701569e-10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7.56964647420946e-09, 1.06153413557165e-07, 7.73161212244915e-08, 2.73083815964487e-09, 0, 0, 0, 3.10873620272248e-08, 4.35955577175346e-07, 3.17525297810897e-07, 1.12151280506804e-08, 0, 0, 0, 7.97940356560808e-09, 1.11899667907275e-07, 8.15013667388112e-08, 2.87866280445344e-09, 0, 0, 0, 7.90880642441617e-15, 1.10909644456321e-13, 8.07802898503338e-14, 2.85319406324002e-15, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2.10193733384667e-09, 8.89957121445723e-08, 7.89423810957122e-07, 1.01362223691026e-06, 0, 0, 0, 4.29357981573436e-07, 1.81789526832161e-05, 0.000161253815049885, 0.000207050320059393, 0, 0, 0, 1.03490196651767e-06, 4.38175943816110e-05, 0.000388677740872647, 0.000499063235327161, 0, 0, 0, 1.52510499567077e-07, 6.45727172734381e-06, 5.72783107472029e-05, 7.35455007312597e-05, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4.12900092569066e-07, 1.74821281258161e-05, 0.000155072731889636, 0.000199113793123596, 0, 0, 0, 8.43421673340291e-05, 0.00357103474249254, 0.0316763559450957, 0.0406725238390073, 0, 0, 0, 0.000203293937880179, 0.00860743490540271, 0.0763510275028770, 0.0980349189037402, 0, 0, 0, 2.99588376756023e-05, 0.00126845270362297, 0.0112516293558755, 0.0144471215059500, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8.45583724479728e-07, 3.58018883466022e-05, 0.000317575560181186, 0.000407767849450346, 0, 0, 0, 0.000172725473470490, 0.00731317069709366, 0.0648704408646246, 0.0832938156486986, 0, 0, 0, 0.000416328424843155, 0.0176272832012257, 0.156360309347564, 0.200767045945447, 0, 0, 0, 6.13531118029043e-05, 0.00259768157178721, 0.0230423650380150, 0.0295864569441068, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1.80248384313223e-07, 7.63168961631566e-06, 6.76958176498084e-05, 8.69216068267225e-05, 0, 0, 0, 3.68189294820547e-05, 0.00155890796404460, 0.0138280714458281, 0.0177552798845543, 0, 0, 0, 8.87464171189200e-05, 0.00375751001925621, 0.0333304583741310, 0.0427963956818956, 0, 0, 0, 1.30783019527325e-05, 0.000553733347413907, 0.00491181292711516, 0.00630678063843913, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3.98275932051420e-05, 0.00135124380234572, 0.00159905700349686, 0.000150220774551341, 0, 0, 0, 0.000944081109407055, 0.0320301490835067, 0.0379043619856881, 0.00356086281096460, 0, 0, 0, 0.000938431591541980, 0.0318384760400942, 0.0376775368029060, 0.00353955409303224, 0, 0, 0, 6.44966084474119e-05, 0.00218819756413582, 0.00258950504260733, 0.000243266783081782, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.000152057766603182, 0.00515891366226354, 0.00610503967363802, 0.000573527889521775, 0, 0, 0, 0.00360440723217381, 0.122287904983828, 0.144715062202607, 0.0135950179923425, 0, 0, 0, 0.00358283793812877, 0.121556116478577, 0.143849066345776, 0.0135136634389483, 0, 0, 0, 0.000246241598970815, 0.00835431939799106, 0.00988647120498635, 0.000928768241998129, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5.81961904767361e-05, 0.00197444121960320, 0.00233654656155943, 0.000219503015517994, 0, 0, 0, 0.00137949395499620, 0.0468025433387562, 0.0553859596450198, 0.00520314268906581, 0, 0, 0, 0.00137123886370600, 0.0465224701521533, 0.0550545220541487, 0.00517200633088269, 0, 0, 0, 9.42426244783592e-05, 0.00319740039492864, 0.00378379200379571, 0.000355462103169780, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4.14913480900040e-06, 0.000140769055937703, 0.000166585589056587, 1.56496085895920e-05, 0, 0, 0, 9.83519082708417e-05, 0.00333681741237359, 0.00394877759541507, 0.000370961402637324, 0, 0, 0, 9.77633562308699e-05, 0.00331684941450152, 0.00392514753932326, 0.000368741515965967, 0, 0, 0, 6.71908849207278e-06, 0.000227960716470149, 0.000269767883161390, 2.53428991392796e-05, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7.87888411084390e-09, 8.38156060964592e-07, 1.93540660594479e-05, 7.06125499387650e-05, 0, 0, 0, 1.96635251839529e-08, 2.09180673062277e-06, 4.83024194819740e-05, 0.000176229480531770, 0, 0, 0, 5.91156161195822e-09, 6.28872201332289e-07, 1.45214414049912e-05, 5.29809086753753e-05, 0, 0, 0, 4.26306687755209e-14, 4.53505254227552e-12, 1.04720004512352e-10, 3.82066823865492e-10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4.70393681392501e-06, 0.000500405018720783, 0.0115549743536490, 0.0421578701386050, 0, 0, 0, 1.17397309952857e-05, 0.00124887313347456, 0.0288380341691014, 0.105214435129378, 0, 0, 0, 3.52938460612765e-06, 0.000375456099808551, 0.00866974838761482, 0.0316312365110550, 0, 0, 0, 2.54518240697841e-11, 2.70756623737281e-09, 6.25210724577196e-08, 2.28105677514248e-07, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1.33650738809755e-05, 0.00142177718582783, 0.0328306038193645, 0.119781168700879, 0, 0, 0, 3.33555441540573e-05, 0.00354836435035478, 0.0819361467847900, 0.298940813721497, 0, 0, 0, 1.00278740725504e-05, 0.00106676571380583, 0.0246329473191320, 0.0898723409007936, 0, 0, 0, 7.23150676878286e-11, 7.69288028976051e-09, 1.77638175334676e-07, 6.48105906444794e-07, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2.35623784406980e-06, 0.000250656692279958, 0.00578797482541135, 0.0211171988432974, 0, 0, 0, 5.88052083701570e-06, 0.000625570082239142, 0.0144451913675084, 0.0527027134081356, 0, 0, 0, 1.76789568061743e-06, 0.000188068825358559, 0.00434274312294936, 0.0158443277344708, 0, 0, 0, 1.27490128898632e-11, 1.35624058872088e-09, 3.13172822688892e-08, 1.14259874455623e-07, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1.39284896058123e-07, 1.52377841278175e-05, 0.000362083023149972, 0.00136149823970918, 0, 0, 0, 2.40939472461594e-06, 0.000263588068278986, 0.00626342805673980, 0.0235516324394591, 0, 0, 0, 8.02392676092680e-06, 0.000877818538123467, 0.0208588852155079, 0.0784332147255826, 0, 0, 0, 3.01166317511719e-06, 0.000329476308105820, 0.00782907650446770, 0.0294387563013863, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4.89724902397190e-07, 5.35759623328497e-05, 0.00127308185015125, 0.00478702006768406, 0, 0, 0, 8.47141814900347e-06, 0.000926774148986861, 0.0220221774260342, 0.0828074057139376, 0, 0, 0, 2.82120808576208e-05, 0.00308640498769923, 0.0733397217887580, 0.275770736908581, 0, 0, 0, 1.05889905957356e-05, 0.00115843682549741, 0.0275269884640660, 0.103506499731134, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1.92009356935888e-07, 2.10058463933462e-05, 0.000499144777359182, 0.00187687544647206, 0, 0, 0, 3.32143932882124e-06, 0.000363365856015726, 0.00863436616190571, 0.0324667923620655, 0, 0, 0, 1.10612784377088e-05, 0.00121010517135436, 0.0287547411813976, 0.108123073987786, 0, 0, 0, 4.15168856011810e-06, 0.000454195220267171, 0.0107926701858428, 0.0405824093379282, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2.44459369202756e-10, 2.67438839483328e-08, 6.35493078885829e-07, 2.38956994095813e-06, 0, 0, 0, 4.22873643308945e-09, 4.62624266696979e-07, 1.09929627341540e-05, 4.13355458688271e-05, 0, 0, 0, 1.40828196740495e-08, 1.54066214052815e-06, 3.66094965525043e-05, 0.000137658387513598, 0, 0, 0, 5.28577972828506e-09, 5.78264928403947e-07, 1.37408373620327e-05, 5.16680558999554e-05, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3.75015592312231e-08, 6.18363759100998e-07, 6.63540432631308e-07, 3.15414951120474e-08, 0, 0, 0, 0.000449336280990438, 0.00740911251451042, 0.00795041050018799, 0.000377923969057945, 0, 0, 0, 0.00159596664251858, 0.0263159173298032, 0.0282385164284108, 0.00134232216168972, 0, 0, 0, 0.000354166447565193, 0.00583985573809210, 0.00626650631756244, 0.000297879328319448, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2.13826961559319e-07, 3.52579589909101e-06, 3.78338494424999e-06, 1.79843777195054e-07, 0, 0, 0, 0.00256203245017490, 0.0422453905732500, 0.0453317716717524, 0.00215485264241539, 0, 0, 0, 0.00909990690828793, 0.150048498212658, 0.161010803033564, 0.00765367294458728, 0, 0, 0, 0.00201939164454985, 0.0332977783862738, 0.0357304611580267, 0.00169845289080267, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1.79225442117069e-07, 2.95525093852036e-06, 3.17115687557557e-06, 1.50741423086842e-07, 0, 0, 0, 0.00214744387355235, 0.0354092334646889, 0.0379961757889262, 0.00180615397944984, 0, 0, 0, 0.00762735824784952, 0.125767621795932, 0.134956004373234, 0.00641515413823324, 0, 0, 0, 0.00169261330593035, 0.0279095255774869, 0.0299485511620927, 0.00142360892213621, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3.29469837125635e-09, 5.43263296705213e-08, 5.82953249803262e-08, 2.77107711527181e-09, 0, 0, 0, 3.94764256066719e-05, 0.000650927359673315, 0.000698483078111053, 3.32024990650858e-05, 0, 0, 0, 0.000140213601926917, 0.00231198413458052, 0.00248089402122564, 0.000117929673605099, 0, 0, 0, 3.11152827207028e-05, 0.000513060352238494, 0.000550543726212613, 2.61701795329060e-05, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5.96381674123622e-07, 2.02286693481147e-05, 0.000141667284538227, 0.000138160139584012, 0, 0, 0, 1.21039296660384e-05, 0.000410553177018604, 0.00287522390849640, 0.00280404426348668, 0, 0, 0, 1.43646093420602e-05, 0.000487233168461080, 0.00341223630308985, 0.00332776226681549, 0, 0, 0, 8.13071032431120e-07, 2.75785554540196e-05, 0.000193140685401633, 0.000188359254159635, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1.90989099187677e-05, 0.000647815904511003, 0.00453684414399223, 0.00442452908057071, 0, 0, 0, 0.000387624020631540, 0.0131478187291128, 0.0920780178320456, 0.0897985151459707, 0, 0, 0, 0.000460021479106397, 0.0156034680434279, 0.109275647797241, 0.106570396983407, 0, 0, 0, 2.60383091562645e-05, 0.000883193379608910, 0.00618526140588654, 0.00603213777963178, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1.96510775980416e-05, 0.000666544879416480, 0.00466800862996920, 0.00455244643107342, 0, 0, 0, 0.000398830600316626, 0.0135279346931153, 0.0947400810406113, 0.0923946757604395, 0, 0, 0, 0.000473321138281433, 0.0160545791683459, 0.112434910870474, 0.109651448678604, 0, 0, 0, 2.67911014779301e-05, 0.000908727341538827, 0.00636408320517860, 0.00620653262902403, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3.08844563278391e-06, 0.000104756984018701, 0.000733643780861296, 0.000715481541833237, 0, 0, 0, 6.26818870172912e-05, 0.00212610685674059, 0.0148897478054145, 0.0145211340915601, 0, 0, 0, 7.43891318497100e-05, 0.00252320488132432, 0.0176707413482998, 0.0172332807760940, 0, 0, 0, 4.21060168045083e-06, 0.000142819393764270, 0.00100020596242855, 0.000975444654228550, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };

    caffe_copy(blob_Y1->count(), &y1[0][0], Y1);
    caffe_copy(blob_Y2->count(), &y2[0][0], Y2);
    caffe_copy(blob_Y2->count(), &y2[0][0], Y2b); //tile
    caffe_copy(blob_Y2->count(), &y2[0][0], Y2b+blob_Y2->count()); //tile
    caffe_copy(blob_Y3->count(), &y3[0][0], Y3);

    blob_top_vec_.push_back(blob_top_);
  }
  virtual ~BSplineBasisLayerTest() {
    delete blob_X1;
    delete blob_X2;
    delete blob_X2b;
    delete blob_X3;

    delete blob_Y1;
    delete blob_Y2;
    delete blob_Y2b;
    delete blob_Y3;

    delete blob_top_;
  }
  Blob<Dtype>* const blob_X1;
  Blob<Dtype>* const blob_X2;
  Blob<Dtype>* const blob_X2b;
  Blob<Dtype>* const blob_X3;

  Blob<Dtype>* const blob_Y1;
  Blob<Dtype>* const blob_Y2;
  Blob<Dtype>* const blob_Y2b;
  Blob<Dtype>* const blob_Y3;

  Blob<Dtype>* const blob_top_;
  vector<Blob<Dtype>*> blob_bottom_vec_;
  vector<Blob<Dtype>*> blob_top_vec_;
};

TYPED_TEST_CASE(BSplineBasisLayerTest, TestDtypesAndDevices);

template <typename Dtype>
static void print_mat(
  const int M, const int N,
  const Dtype* Z, const char* name)
{
  std::cout<<name<<"("<<M<<"x"<<N<<"):"<<std::endl;
  for(int m=0; m<M; ++m) {
    for(int n=0; n<N; ++n) {
      std::cout<<Z[m*N+n]<<" ";
    }
    std::cout<<std::endl<<std::flush;
  }
}

template <typename Dtype>
static void check_all_eq(
  const int N,
  const Dtype* calc, const Dtype* expect)
{
  for(int i=0; i<N; ++i)
  {
    EXPECT_EQ(calc[i], expect[i]);
  }
}

template <>
void check_all_eq(
  const int N,
  const float* calc, const float* expect)
{
  for(int i=0; i<N; ++i)
  {
    EXPECT_NEAR(calc[i], expect[i], 1e-6);
  }
}

template <>
void check_all_eq(
  const int N,
  const double* calc, const double* expect)
{
  for(int i=0; i<N; ++i)
  {
    EXPECT_NEAR(calc[i], expect[i], 1e-8);
  }
}

TYPED_TEST(BSplineBasisLayerTest, TestForwardY1) {
  typedef typename TypeParam::Dtype Dtype;
  this->blob_bottom_vec_.clear();
  this->blob_bottom_vec_.push_back(this->blob_X1);
  bool IS_VALID_CUDA = false;
#ifndef CPU_ONLY
  IS_VALID_CUDA = CAFFE_TEST_CUDA_PROP.major >= 2;
#endif
  if (Caffe::mode() == Caffe::CPU) {
    LayerParameter layer_param;
    layer_param.mutable_bspline_basis_param()->set_degree(3);
    layer_param.mutable_bspline_basis_param()->add_knot_vector("0,0,0,0,0.25,0.5,0.75,1,1,1,1");
    shared_ptr<BSplineBasisLayer<Dtype> > layer(
        new BSplineBasisLayer<Dtype>(layer_param));
    layer->SetUp(this->blob_bottom_vec_, this->blob_top_vec_);
    EXPECT_EQ(this->blob_top_->num_axes(), 2);
    EXPECT_EQ(this->blob_top_->shape(0), this->blob_Y1->shape(0));
    EXPECT_EQ(this->blob_top_->shape(1), this->blob_Y1->shape(1));
    layer->Forward(this->blob_bottom_vec_, this->blob_top_vec_);
    const Dtype* Y1_calc = this->blob_top_->cpu_data();
    const Dtype* Y1 = this->blob_Y1->cpu_data();
    check_all_eq(this->blob_Y1->count(), Y1_calc, Y1);
  } else {
    LOG(INFO) << "GPU version not implemented.";
  }
}

TYPED_TEST(BSplineBasisLayerTest, TestForwardY2) {
  typedef typename TypeParam::Dtype Dtype;
  this->blob_bottom_vec_.clear();
  this->blob_bottom_vec_.push_back(this->blob_X2);
  bool IS_VALID_CUDA = false;
#ifndef CPU_ONLY
  IS_VALID_CUDA = CAFFE_TEST_CUDA_PROP.major >= 2;
#endif
  if (Caffe::mode() == Caffe::CPU) {
    LayerParameter layer_param;
    layer_param.mutable_bspline_basis_param()->set_degree(3);
    layer_param.mutable_bspline_basis_param()->add_knot_vector("0,0,0,0,0.25,0.5,0.75,1,1,1,1");
    layer_param.mutable_bspline_basis_param()->add_knot_vector("0,0,0,0,0.25,0.5,0.75,1,1,1,1");
    shared_ptr<BSplineBasisLayer<Dtype> > layer(
        new BSplineBasisLayer<Dtype>(layer_param));
    layer->SetUp(this->blob_bottom_vec_, this->blob_top_vec_);
    EXPECT_EQ(this->blob_top_->num_axes(), 2);
    EXPECT_EQ(this->blob_top_->shape(0), this->blob_Y2->shape(0));
    EXPECT_EQ(this->blob_top_->shape(1), this->blob_Y2->shape(1));
    layer->Forward(this->blob_bottom_vec_, this->blob_top_vec_);
    const Dtype* Y2_calc = this->blob_top_->cpu_data();
    const Dtype* Y2 = this->blob_Y2->cpu_data();
    check_all_eq(this->blob_Y2->count(), Y2_calc, Y2);
  } else {
    LOG(INFO) << "GPU version not implemented.";
  }
}

TYPED_TEST(BSplineBasisLayerTest, TestForwardY2b) {
  typedef typename TypeParam::Dtype Dtype;
  this->blob_bottom_vec_.clear();
  this->blob_bottom_vec_.push_back(this->blob_X2b);
  bool IS_VALID_CUDA = false;
#ifndef CPU_ONLY
  IS_VALID_CUDA = CAFFE_TEST_CUDA_PROP.major >= 2;
#endif
  if (Caffe::mode() == Caffe::CPU) {
    LayerParameter layer_param;
    layer_param.mutable_bspline_basis_param()->set_degree(3);
    layer_param.mutable_bspline_basis_param()->add_knot_vector("0,0,0,0,0.25,0.5,0.75,1,1,1,1");
    layer_param.mutable_bspline_basis_param()->add_knot_vector("0,0,0,0,0.25,0.5,0.75,1,1,1,1");
    shared_ptr<BSplineBasisLayer<Dtype> > layer(
        new BSplineBasisLayer<Dtype>(layer_param));
    layer->SetUp(this->blob_bottom_vec_, this->blob_top_vec_);
    EXPECT_EQ(this->blob_top_->num_axes(), 3);
    EXPECT_EQ(this->blob_top_->shape(0), this->blob_Y2b->shape(0));
    EXPECT_EQ(this->blob_top_->shape(1), this->blob_Y2b->shape(1));
    EXPECT_EQ(this->blob_top_->shape(2), this->blob_Y2b->shape(2));
    layer->Forward(this->blob_bottom_vec_, this->blob_top_vec_);
    const Dtype* Y2b_calc = this->blob_top_->cpu_data();
    const Dtype* Y2b = this->blob_Y2b->cpu_data();
    check_all_eq(this->blob_Y2b->count(), Y2b_calc, Y2b);
  } else {
    LOG(INFO) << "GPU version not implemented.";
  }
}

TYPED_TEST(BSplineBasisLayerTest, TestForwardY3) {
  typedef typename TypeParam::Dtype Dtype;
  this->blob_bottom_vec_.clear();
  this->blob_bottom_vec_.push_back(this->blob_X3);
  bool IS_VALID_CUDA = false;
#ifndef CPU_ONLY
  IS_VALID_CUDA = CAFFE_TEST_CUDA_PROP.major >= 2;
#endif
  if (Caffe::mode() == Caffe::CPU) {
    LayerParameter layer_param;
    layer_param.mutable_bspline_basis_param()->set_degree(3);
    layer_param.mutable_bspline_basis_param()->add_knot_vector("0,0,0,0,0.25,0.5,0.75,1,1,1,1");
    layer_param.mutable_bspline_basis_param()->add_knot_vector("0,0,0,0,0.25,0.5,0.75,1,1,1,1");
    layer_param.mutable_bspline_basis_param()->add_knot_vector("0,0,0,0,0.25,0.5,0.75,1,1,1,1");
    shared_ptr<BSplineBasisLayer<Dtype> > layer(
        new BSplineBasisLayer<Dtype>(layer_param));
    layer->SetUp(this->blob_bottom_vec_, this->blob_top_vec_);
    EXPECT_EQ(this->blob_top_->num_axes(), 2);
    EXPECT_EQ(this->blob_top_->shape(0), this->blob_Y3->shape(0));
    EXPECT_EQ(this->blob_top_->shape(1), this->blob_Y3->shape(1));
    layer->Forward(this->blob_bottom_vec_, this->blob_top_vec_);
    const Dtype* Y3_calc = this->blob_top_->cpu_data();
    const Dtype* Y3 = this->blob_Y3->cpu_data();
    check_all_eq(this->blob_Y3->count(), Y3_calc, Y3);
  } else {
    LOG(INFO) << "GPU version not implemented.";
  }
}

}  // namespace caffe