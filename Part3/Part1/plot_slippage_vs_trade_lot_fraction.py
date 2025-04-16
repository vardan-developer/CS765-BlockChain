import matplotlib.pyplot as plt

def get_slippage(k : float) -> float:
    fees = 0.003
    return (((1-fees)/(1+(1-fees)*k)) - 1)*100

def main():
    trade_lot_fractions = [i/100 for i in range(100)]
    slippage = [get_slippage(k) for k in trade_lot_fractions]
    plt.plot(trade_lot_fractions, slippage)
    plt.xlabel('Trade Lot Fraction')
    plt.ylabel('Slippage (%)')
    plt.title('Slippage vs Trade Lot Fraction')
    plt.savefig('slippage_vs_trade_lot_fraction.png')

if __name__ == "__main__":
    main()