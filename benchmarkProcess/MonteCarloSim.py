import numpy as np



def monte_carlo_var(num_simulations, num_days, initial_price, daily_volatility, confidence_level):
    # Generate random daily returns
    daily_returns = np.random.normal(0, daily_volatility, (num_simulations, num_days))

    # Calculate price paths
    price_paths = initial_price * np.exp(np.cumsum(daily_returns, axis=1))

    # Calculate the final price for each simulation
    final_prices = price_paths[:, -1]

    # Sort the final prices
    sorted_final_prices = np.sort(final_prices)

    # Calculate VaR
    var_index = int(num_simulations * (1 - confidence_level))
    var = initial_price - sorted_final_prices[var_index]

    return var

