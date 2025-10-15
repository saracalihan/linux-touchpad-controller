export type ButtonProps = {
  children: React.ReactNode;
  className?: string;
  fullWidth?: boolean;
  secondary?: boolean;
  disabled?: boolean;
  onClick?: () => any
};


export const Button = ({ children, onClick, className,disabled = false, secondary = false, fullWidth = false }: ButtonProps) => {
  let color = "white";//'var(--bg-color)';
  let background = 'var(--color)';

  if(secondary){
    color = 'var(--color)';
    background = 'transparent';

  }

  return <button className={className} onClick={onClick} style={{
    color, background, width: fullWidth ? '100%' : 'fit-content', border: '1px solid var(--color)', cursor: disabled ? 'not-allowed' : 'pointer',
  }} disabled={disabled}>{children}</button>
}