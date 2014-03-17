function r=displayObst(N,X,Y,u)
% display solution
for (i=1:N)
	U(i,1:N) = u((i-1)*N+1:i*N);
end

figure(1)
r=surf(X,Y,U);
