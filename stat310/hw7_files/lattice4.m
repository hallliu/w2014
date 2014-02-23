function V = lattice4(nincr)
    a = linspace(0, 3, nincr);
    V = zeros(4, nincr^4);
    for i = 1:nincr
        for j = 1:nincr
            for k = 1:nincr
                for l = 1:nincr
                    V(:, (i-1)*nincr^3+(j-1)*nincr^2+(k-1)*nincr+l) = [a(i);a(j);a(k);a(l)];
                end
            end
        end
    end                
end