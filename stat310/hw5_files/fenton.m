function y=fenton(x);
% this is fenton's function, which can trip Newton's method
% if it is started at [3,2] it convergence
% and f it is started at [3,4] it diverges
% example usage: xout=fminunc(@fenton,[3 2]',[])
% example usage: [xout,iteratesGradNorms]=newtonLikeMethod(@fenton_wrap,[3 2]',1,1e-9,100)
% example usage [xc,histout,costdata]=cgtrust([3 4]',@fenton_fg,1e-10)
x1=x(1); x2=x(2);
y=12+x1*x1+(1+x2*x2)/(x1*x1)+(x1*x1*x2*x2+100)/((x1*x2)^4);
y=y*1/10;