function [tab2d] = transitionTab(tab3d,numBalls)
%UNTITLED3 Summary of this function goes here
%   Detailed explanation goes here
    tab2d = zeros(numBalls,numBalls+1);
    for i=1:numBalls
        for j=0:i
            tab2d(i,j+1) = sum(tab3d(i,:,i-j+1));
        end
    end
end

