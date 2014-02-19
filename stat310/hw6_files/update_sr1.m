function B1 = update_sr1(B, s, y)
    ymbs = y - B * s;
    if abs(s.' * ymbs) < 1e-8 * norm(s) * norm(ymbs)
        B1 = B;
        return;
    end
    
    B1 = (ymbs * ymbs.') / (ymbs.' * s);
end

