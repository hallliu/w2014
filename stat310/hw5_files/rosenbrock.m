function y = rosenbrock(x)
    N = length(x) / 2;
    I = 1:N;
    y = sum((1 - x(2*I-1)).^2) + 10*sum((x(2*I) - x(2*I-1).^2).^2);
end
    
    