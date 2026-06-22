import "./globals.css";

export const metadata = {
  title: "TradeCore",
  description: "Low-latency exchange matching engine demo",
};

export default function RootLayout({ children }: { children: React.ReactNode }) {
  return (
    <html lang="en">
      <body>{children}</body>
    </html>
  );
}