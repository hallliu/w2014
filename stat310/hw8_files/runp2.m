ems = [10, 20, 40, 80];
eps = [1e-2, 1e-3,1e-4];
iter_cts = zeros(4, 3);
y = randn(300, 1);
for i=1:4
    for j = 1:3
        [~, iter_cts(i, j), ~] = p2(@rosenbrock, y, 0.5, 1e-3, ems(i), eps(j));
    end
end