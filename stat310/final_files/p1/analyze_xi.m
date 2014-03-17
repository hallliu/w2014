function rs = analyze_xi(xi)
    N = length(xi);
    rs = 1:N-1;
    for i = rs
        rs(i) = norm(xi{1, i+1} - xi{1, N}) / norm(xi{1, i} - xi{1, N});
    end
end