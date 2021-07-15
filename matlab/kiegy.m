clearvars stdcb edens
%stdcb(1:55) = 2.5;
%stdcb = transpose(stdcb);
%for i=1:55
%   stdcb(i) = (rand(1) - 0.5)*1 ;
%end
%edens(1:36) = 1/2.25;
%edens = transpose(edens);
%for i=1:36
%   edens(i) =  1/2.25 + (rand(1) - 0.5) * 0.08;
%end
%ps = cat(1, edens, stdcb);
%disp(ps);

%error('sajt');

clearvars meres;
A = dlmread('~/work/gps/IonosphereModeler/bin/alakMatrix');
allandok = dlmread('~/work/gps/IonosphereModeler/bin/allandok');
%meres = A*ps + allandok;
%meres = transpose(meres);
%for i=1:552
%   meres(i) = meres(i) + (rand(1)-0.5)*1;
%end
paramStart = dlmread('~/work/gps/IonosphereModeler/bin/paramStartVektor');
paramJav = dlmread('~/work/gps/IonosphereModeler/bin/paramJavVektor');
meres = dlmread('~/work/gps/IonosphereModeler/bin/meresVektor');
meresJav = dlmread('~/work/gps/IonosphereModeler/bin/meresJavVektor');

normalmat = transpose(A)*A;
npinv = inv(normalmat);

for j=1:100
    a0 = A * paramStart;
    ttv = meres - a0 - allandok;
    paramJav = npinv * transpose(A) * ttv;
    meresJav = A * paramJav - ttv;
    paramStart = paramStart + paramJav;
end

disp(paramStart);
%disp(meres);

%disp(meres+meresJav - A*paramStart + allandok)