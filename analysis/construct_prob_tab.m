function [tab3d] = construct_prob_tab(n,m)
% Construct the 3D-probability table given the number of balls and bins
% tab(x,a,b) is the probability that x balls are thrown into n bins then
% a bins are empty and (b-1) bins have exactly 1 ball
%            x<=m   n-m<=a<n   b-1<=min(n,m)=x (assume m<n)
% tab(x,a,b) = tab(x-1,a+1,b-1)*(a+1)/n + tab(x-1,a,b+1)*b/n +
%                tab(x-1,a,b)*(n-a-b+1)/n
% tab(1,a,b) = 1 iff a=n-1 and b=2
    tab3d = zeros(m,n,m+1);
    tab3d(1,n-1,2) = 1;
    for x=2:m
        for a=(n-m):n-1
            for b=1:x+1
                if b==1
                    tab3d(x,a,b)=tab3d(x-1,a,b+1)*b/n+tab3d(x-1,a,b)*(n-a-b+1)/n;
                else if b==m+1
                        tab3d(x,a,b)=tab3d(x-1,a+1,b-1)*(a+1)/n+tab3d(x-1,a,b)*(n-a-b+1)/n;
                    else
                        tab3d(x,a,b)=tab3d(x-1,a+1,b-1)*(a+1)/n+tab3d(x-1,a,b+1)*b/n+tab3d(x-1,a,b)*(n-a-b+1)/n;
                    end
                end
            end
        end
    end
end

