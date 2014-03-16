function J = enso_jac(b)
    xs = 1:168;
    J = zeros(168, 9);
    for x=xs
        J(x, :) = grad_resid(x, b);
    end
end

function g = grad_resid(x, b)
    g = zeros(1, 9);
    tpx = 2*pi*x;
    g(1) = 1;
    g(2) = cos(tpx/12);
    g(3) = sin(tpx/12);
    g(4) = tpx * (b(6)*cos(tpx*b(4)) - b(5)*sin(tpx*b(4)));
    g(5) = cos(tpx*b(4));
    g(6) = sin(tpx*b(4));
    g(7) = tpx * (b(9)*cos(tpx*b(7)) - b(8)*sin(tpx*b(7)));
    g(8) = cos(tpx*b(7));
    g(9) = sin(tpx*b(7));
end

