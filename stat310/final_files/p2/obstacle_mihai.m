%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% This program solves the obstacle problem
% 
%               -div(grad(u)) >= f
%                           u >= g
%    (div(grad(u)) + f)(u - g) = 0
%
% on the 2D unit square for homogeneous Dirichlet 
% boundary conditions with finite element discretisation. 
% Based on scriptfiles by 
% Author: Stefan Hüeber
% modified by Mihai
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [A,g,f,X,Y]=obstacle_mihai(N)
% function [A,g,f,X,Y]=obstacle_mihai(N)
% input: size of mesh in one direction
% output: matrix A, right hand side f, obstacle height f, 
% mesh coordinates X,Y
% one direction N

%%%% User specified Parameters %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% N = 24; % number of discretisation points in each direction
ffunc = inline('-16*x.*(1-x).*y.*(1-y)','x','y'); % right hand side f
gfunc = inline('-0.008*(1+2*x+2*y)','x','y');     % obstacle function g
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

h = 1/(N-1);
[X,Y] = meshgrid(0:h:1); % discretisation of unit square

% Assemble finite element matrix
A = gallery('poisson',N); % system matrix

% Assemble right hand side
ff = h*h*(ffunc(X,Y)); 
ft = ff';
f = ft(:); 

% Discrete obstacle values
gh = gfunc(X,Y);
gt = gh';
g = gt(:); 

% Insert Dirichlet datas on the whole boundary
for (i=1:N)
	A(i,:)   = zeros(1,N*N); A(i,i)     = 1; f(i)   = 0;
	ind = i+(N-1)*N;
	A(ind,:) = zeros(1,N*N); A(ind,ind) = 1; f(ind) = 0;
	ind = (i-1)*N+1;
	A(ind,:) = zeros(1,N*N); A(ind,ind) = 1; f(ind) = 0;
	ind = i*N;
	A(ind,:) = zeros(1,N*N); A(ind,ind) = 1; f(ind) = 0;
end

% initial values
uold = zeros(N*N,1);
lambda = zeros(N*N,1);
u = uold;
it = 0;

