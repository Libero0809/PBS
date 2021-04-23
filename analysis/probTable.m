setSize = 1000000;
errorRate = 0.02;
numErrors = setSize*errorRate;
avgErrorPerGroup = 10;
numGroups = numErrors/avgErrorPerGroup;
n=255;
m=254;
r=3;
tab3d = construct_prob_tab(n,m);
tab2d = transitionTab(tab3d,m);
rTable = roundTab(tab2d,r);
probFailOneGroup = 0;
for i=0:m-1
    probFailOneGroup = probFailOneGroup+exp(-avgErrorPerGroup)*avgErrorPerGroup^i/factorial(i)*rTable(i+1,r);
end
2*(1-(1-probFailOneGroup)^numGroups)
expRemainError = 0;
for i=1:21
    expRemainError = expRemainError+tab2d(20,i)*(i-1);
end