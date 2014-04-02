hr = 60; %bpm
time = 3000; %sec
sampling_rate = .1; %s/sample
sr = sampling_rate; % samples/min
X = 0:sr:time; % array (seconds)
X=X'
ph1=.25;
ph2=.25;
ph3=.25;
y1 = sin(X./60.*hr/(pi*2));
y2 = sin((X-ph1)./60.*hr/(pi*2));
y3 = sin((X-ph2)./60.*hr/(pi*2));
y4 = sin((X-ph3)./60.*hr/(pi*2));
[X,y1,y2,y3,y4]
csvwrite('sim_vals.csv' , [X,y1,y2,y3,y4]);