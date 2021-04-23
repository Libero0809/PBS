function [roundTab] = roundTab(tab2d,maxRound)
%UNTITLED4 Summary of this function goes here
%   Detailed explanation goes here
    numBalls = size(tab2d,1);
    roundTab = zeros(numBalls,maxRound);
    transTab = zeros(numBalls+1,numBalls+1);
    transTab(2:(numBalls+1),:) = tab2d;
    transTab(1,1) = 1;
    for round=1:maxRound
        roundTab(:,round) = transTab(2:(numBalls+1),1);
        transTab = transTab*transTab;
    end
    for i=1:size(roundTab,1)
        for j=1:size(roundTab,2)
            roundTab(i,j) = 1-roundTab(i,j);
        end
    end
end

