function test_gradient()
    X = lattice3(10);
    theta1 = [0.1; 0.1; 0.1];
    theta2 = 10 * theta1;
    
    y1 = sample_from_matern(X, theta1, 1);
    y2 = sample_from_matern(X, theta2, 1);
    
    [fy1, gy1] = matern_fn(X, y1, 2*theta1);
    [fy2, gy2] = matern_fn(X, y2, 2*theta2);
    
    id = eye(3);
    gy1_fdiff = 1:3;
    for i = 1:3
        [fy1_pert, ~] = matern_fn(X, y1, 2*theta1 + id(:, i)*10e-8);
        gy1_fdiff(i) = (fy1_pert - fy1) / 10e-8;
    end
    
    gy2_fdiff = 1:3;
    for i = 1:3
        [fy2_pert, ~] = matern_fn(X, y2, 2*theta2 + id(:, i)*10e-8);
        gy2_fdiff(i) = (fy2_pert - fy2) / 10e-8;
    end

end

% Returns a lattice of points in [0,3]^3
function V = lattice3(nincr)
    a = linspace(0, 3, nincr);
    V = zeros(3, nincr^3);
    for i = 1:nincr
        for j = 1:nincr
            for k = 1:nincr
                V(:, (i-1)*nincr^2+(j-1)*nincr+k) = [a(i);a(j);a(k)];
            end
        end
    end                
end