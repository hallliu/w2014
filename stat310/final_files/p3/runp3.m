start_pt = randn(10124, 1);
sizes = [200, 400, 600, 800, 1000, 1500, 2000];
times = zeros(1, 7);
points = cell(1, 7);
for i = 1:length(sizes)
    X = MatrixMat(1:sizes(i), 2:124);
    y = MatrixMat(1:sizes(i), 1);
    s = start_pt(1:sizes(i)*5+124);
    tic;
    points{i} = p3_int_point(X, y, s, 200, 1e-3);
    times(i) = toc;
end